import pyfasterac as pyf
import ROOT
import array
import os
import math

# ------------------------
# Calibration
# ------------------------
calibration = {
    1: lambda q: 0.001860 * q - 33.117,
    2: lambda q: 0.001638 * q - 18.51
}

# ------------------------
# Parameters
# ------------------------
angles = range(0, 181, 15)
file_template = "compton_{angle}_Na-22-colimated.fast/compton_{angle}_Na-22-colimated_0001.fast"
out_dir = "../output"
os.makedirs(out_dir, exist_ok=True)

histogram_data_file = os.path.join(out_dir, "histogram_data.root")
box_size = 20

# ------------------------
# Functions
# ------------------------
def build_histogram(file_path, nbins=200, e_max=2000):
    h2 = ROOT.TH2D("coinc", "Detector1 - Detector2;E1 (keV);E2 (keV)",
                   nbins, 0, e_max, nbins, 0, e_max)
    reader = pyf.fastreader(file_path)
    while reader.get_next_event():
        event = reader.get_event()
        if event.multiplicity < 2:
            continue
        E1_list, E2_list = [], []
        for sub_event in event.sub_events:
            det_id = sub_event.label % 1000
            E = calibration.get(det_id, lambda q: 0)(sub_event.q)
            if det_id == 1:
                E1_list.append(E)
            elif det_id == 2:
                E2_list.append(E)
        for E1 in E1_list:
            for E2 in E2_list:
                if ((E1 + E2) > 1400.) or ( (E1+E2) < 1000.):
                    continue
                h2.Fill(E1, E2)
    return h2

def analyze_peak(hist2d, box_size=20):
    if hist2d.GetEntries() < 10:
        return None
    max_bin = hist2d.GetMaximumBin()
    binx, biny, binz = array.array('i',[0]), array.array('i',[0]), array.array('i',[0])
    hist2d.GetBinXYZ(max_bin, binx, biny, binz)
    peak_bin_x, peak_bin_y = binx[0], biny[0]

    sum_w = sum_x = sum_y = sum_x2 = sum_y2 = 0.0
    for ix in range(max(1, peak_bin_x - box_size), min(hist2d.GetNbinsX()+1, peak_bin_x + box_size + 1)):
        for iy in range(max(1, peak_bin_y - box_size), min(hist2d.GetNbinsY()+1, peak_bin_y + box_size + 1)):
            w = hist2d.GetBinContent(ix, iy)
            if w == 0: continue
            x = hist2d.GetXaxis().GetBinCenter(ix)
            y = hist2d.GetYaxis().GetBinCenter(iy)
            sum_w += w; sum_x += w*x; sum_y += w*y
            sum_x2 += w*x*x; sum_y2 += w*y*y

    mu_x = sum_x/sum_w
    mu_y = sum_y/sum_w
    sigma_x = math.sqrt(sum_x2/sum_w - mu_x**2)
    sigma_y = math.sqrt(sum_y2/sum_w - mu_y**2)
    return mu_x, mu_y, sigma_x, sigma_y, peak_bin_x, peak_bin_y

# ------------------------
# Main processing loop
# ------------------------
output_file = ROOT.TFile(histogram_data_file, "RECREATE")

# Create a tree to store metadata and initial parameters
metadata_tree = ROOT.TTree("metadata", "Histogram metadata and initial parameters")

# Create branches for storing data
angle_val = array.array('i', [0])
n_bins = array.array('i', [0])
mu_x_guess = array.array('d', [0.0])
mu_y_guess = array.array('d', [0.0])
sigma_x_guess = array.array('d', [0.0])
sigma_y_guess = array.array('d', [0.0])
maximum_val = array.array('d', [0.0])
x_min_fit = array.array('d', [0.0])
x_max_fit = array.array('d', [0.0])
y_min_fit = array.array('d', [0.0])
y_max_fit = array.array('d', [0.0])

metadata_tree.Branch("angle", angle_val, "angle/I")
metadata_tree.Branch("n_bins", n_bins, "n_bins/I")
metadata_tree.Branch("mu_x_guess", mu_x_guess, "mu_x_guess/D")
metadata_tree.Branch("mu_y_guess", mu_y_guess, "mu_y_guess/D")
metadata_tree.Branch("sigma_x_guess", sigma_x_guess, "sigma_x_guess/D")
metadata_tree.Branch("sigma_y_guess", sigma_y_guess, "sigma_y_guess/D")
metadata_tree.Branch("maximum", maximum_val, "maximum/D")
metadata_tree.Branch("x_min_fit", x_min_fit, "x_min_fit/D")
metadata_tree.Branch("x_max_fit", x_max_fit, "x_max_fit/D")
metadata_tree.Branch("y_min_fit", y_min_fit, "y_min_fit/D")
metadata_tree.Branch("y_max_fit", y_max_fit, "y_max_fit/D")

# Create trees for storing bin data for each angle
bin_trees = {}
bin_data = {}

processed_count = 0

for angle in angles:
    file_path = file_template.format(angle=angle)
    if not os.path.exists(file_path):
        print(f"Missing {file_path}, skipping.")
        continue

    print(f"Reading data for angle {angle}° ...")
    h2 = build_histogram(file_path)
    guess = analyze_peak(h2, box_size)
    if guess is None: 
        print(f"No peak found for angle {angle}°, skipping.")
        continue
    
    mu_x, mu_y, sigma_x, sigma_y, peak_bin_x, peak_bin_y = guess

    # Define fit window
    fit_box = max(5, box_size//2)
    x_min = h2.GetXaxis().GetBinLowEdge(max(1, peak_bin_x - fit_box))
    x_max = h2.GetXaxis().GetBinUpEdge(min(h2.GetNbinsX(), peak_bin_x + fit_box))
    y_min = h2.GetYaxis().GetBinLowEdge(max(1, peak_bin_y - fit_box))
    y_max = h2.GetYaxis().GetBinUpEdge(min(h2.GetNbinsY(), peak_bin_y + fit_box))

    # Store the 2D histogram
    h2.SetName(f"hist2d_{angle}")
    h2.SetTitle(f"Coincidence histogram for {angle}°")
    h2.SetDirectory(output_file)
    h2.Write()

    # Extract histogram data in fit window, ignore low counts
    bin_count = 0
    for ix in range(1, h2.GetNbinsX()+1):
        x_val = h2.GetXaxis().GetBinCenter(ix)
        if x_val < x_min or x_val > x_max: continue
        for iy in range(1, h2.GetNbinsY()+1):
            y_val = h2.GetYaxis().GetBinCenter(iy)
            if y_val < y_min or y_val > y_max: continue
            h_val = h2.GetBinContent(ix, iy)
            if h_val < 5:  # ignore very low counts
                continue
            bin_count += 1

    # Create a tree for this angle's bin data
    bin_tree_name = f"bins_{angle}"
    bin_tree = ROOT.TTree(bin_tree_name, f"Bin data for angle {angle}°")
    
    # Branches for bin data
    x_val_arr = array.array('d', [0.0])
    y_val_arr = array.array('d', [0.0])
    h_val_arr = array.array('d', [0.0])
    
    bin_tree.Branch("x", x_val_arr, "x/D")
    bin_tree.Branch("y", y_val_arr, "y/D")
    bin_tree.Branch("h", h_val_arr, "h/D")

    # Fill the bin tree
    for ix in range(1, h2.GetNbinsX()+1):
        x_val = h2.GetXaxis().GetBinCenter(ix)
        if x_val < x_min or x_val > x_max: continue
        for iy in range(1, h2.GetNbinsY()+1):
            y_val = h2.GetYaxis().GetBinCenter(iy)
            if y_val < y_min or y_val > y_max: continue
            h_val = h2.GetBinContent(ix, iy)
            if h_val < 5:  # ignore very low counts
                continue
            
            x_val_arr[0] = x_val
            y_val_arr[0] = y_val
            h_val_arr[0] = h_val
            bin_tree.Fill()

    bin_tree.Write()

    # Fill metadata tree
    angle_val[0] = angle
    n_bins[0] = bin_count
    mu_x_guess[0] = mu_x
    mu_y_guess[0] = mu_y
    sigma_x_guess[0] = sigma_x
    sigma_y_guess[0] = sigma_y
    maximum_val[0] = h2.GetMaximum()
    x_min_fit[0] = x_min
    x_max_fit[0] = x_max
    y_min_fit[0] = y_min
    y_max_fit[0] = y_max
    metadata_tree.Fill()
    
    print(f"Angle {angle}° processed. Found {bin_count} bins for fitting.")
    processed_count += 1

# Write metadata tree
metadata_tree.Write()
output_file.Close()

print(f"Histogram data saved to {histogram_data_file}")
print(f"Processed {processed_count} angles successfully.")