import pyfasterac as pyf
import ROOT
import array
import math

# --- Calibration for detectors ---
calibration = {
    1: lambda q: 0.001841*q - 31.41,
    2: lambda q: 0.001628*q - 16.36
}

# --- Build 2D histogram ---
def build_histogram(file_path, nbins=200, e_max=2000):
    h2 = ROOT.TH2D("coinc_45", "Detector1 vs Detector2 at 45 deg;E1 (keV);E2 (keV)",
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
                h2.Fill(E1, E2)

    return h2

# --- Moment-based peak analysis ---
def analyze_peak(hist2d, box_size=20):
    if hist2d.GetEntries() == 0:
        print("Histogram is empty, cannot analyze")
        return None

    # Find max bin
    max_bin = hist2d.GetMaximumBin()
    binx = array.array('i', [0])
    biny = array.array('i', [0])
    binz = array.array('i', [0])
    hist2d.GetBinXYZ(max_bin, binx, biny, binz)

    # Peak coordinates
    peak_bin_x, peak_bin_y = binx[0], biny[0]
    peak_x = hist2d.GetXaxis().GetBinCenter(peak_bin_x)
    peak_y = hist2d.GetYaxis().GetBinCenter(peak_bin_y)

    # Compute moments in a small box around the peak
    sum_w = sum_x = sum_y = sum_x2 = sum_y2 = 0.0
    for ix in range(max(1, peak_bin_x - box_size), min(hist2d.GetNbinsX() + 1, peak_bin_x + box_size + 1)):
        for iy in range(max(1, peak_bin_y - box_size), min(hist2d.GetNbinsY() + 1, peak_bin_y + box_size + 1)):
            w = hist2d.GetBinContent(ix, iy)
            if w == 0:
                continue
            x = hist2d.GetXaxis().GetBinCenter(ix)
            y = hist2d.GetYaxis().GetBinCenter(iy)
            sum_w += w
            sum_x += w * x
            sum_y += w * y
            sum_x2 += w * x**2
            sum_y2 += w * y**2

    mu_x = sum_x / sum_w
    mu_y = sum_y / sum_w
    sigma_x = (sum_x2 / sum_w - mu_x**2)**0.5
    sigma_y = (sum_y2 / sum_w - mu_y**2)**0.5

    print("\nMoment-based peak analysis for 45°:")
    print(f"  Centroid: x = {mu_x:.2f} keV, y = {mu_y:.2f} keV")
    print(f"  Width: sigma_x = {sigma_x:.2f} keV, sigma_y = {sigma_y:.2f} keV")

    return mu_x, mu_y, sigma_x, sigma_y, peak_bin_x, peak_bin_y

# Main 
file_path = "compton_45_Na-22_colimated.fast/compton_45_Na-22_colimated_0001.fast"
out_file = ROOT.TFile("coincidence_45_moments.root", "RECREATE")
#Defining the Bi-Gaussian 



# Build 2D histogram
hist2d = build_histogram(file_path)
hist2d.SetDirectory(out_file)
hist2d.Write()

# Analyze peak
mu_x, mu_y, sigma_x, sigma_y, peak_bin_x, peak_bin_y = analyze_peak(hist2d, box_size=20)

# Draw histogram with overlay
c = ROOT.TCanvas("c", "Moment Analysis 45", 800, 600)
hist2d.Draw("COLZ")

# Draw box used for moment calculation
x_min = hist2d.GetXaxis().GetBinLowEdge(max(1, peak_bin_x - 20))
x_max = hist2d.GetXaxis().GetBinUpEdge(min(hist2d.GetNbinsX(), peak_bin_x + 20))
y_min = hist2d.GetYaxis().GetBinLowEdge(max(1, peak_bin_y - 20))
y_max = hist2d.GetYaxis().GetBinUpEdge(min(hist2d.GetNbinsY(), peak_bin_y + 20))
box = ROOT.TBox(x_min, y_min, x_max, y_max)
box.SetLineColor(ROOT.kRed)
box.SetLineWidth(2)
box.SetFillStyle(0)
box.Draw("SAME")

# Draw ellipse representing sigma_x and sigma_y
ellipse = ROOT.TEllipse(mu_x, mu_y, sigma_x, sigma_y)
ellipse.SetLineColor(ROOT.kBlue)
ellipse.SetLineWidth(2)
ellipse.SetFillStyle(0)
ellipse.Draw("SAME")
# Guassian + Background Fit
# Create TF2 from the rotated Gaussian
#f2 = ROOT.TF2("rot_gauss", rotated_gaussian, 0, 200000, 0, 200000, 6)
f2 = ROOT.TF2(
    "rot_gaus",
    "[0] * exp( -0.5 * ( ( ( (x-[1])*cos([6]) + (y-[3])*sin([6]) ) / [2] )**2 + ( ( -(x-[1])*sin([6]) + (y-[3])*cos([6]) ) / [4] )**2 ) ) + [5]+x*[7]+y*[8]",
    x_min, x_max, y_min, y_max
)

#  Set initial parameters from moments 
f2.SetParameter(0, hist2d.GetMaximum())   # Amplitude
f2.SetParameter(1, mu_x)                  # x0
f2.SetParameter(3, mu_y)                  # y0
f2.SetParameter(2, sigma_x)               # sigma_x
f2.SetParameter(4, sigma_y)               # sigma_y
f2.SetParameter(6, math.pi/4)             #theta(rad)
f2.SetParameter(7,0)
f2.SetParameter(8,0)
f2.SetParLimits(0, 0, 1e9)
# Fit the histogram
hist2d.Fit(f2, "R")
# Draw the fit as contour over the 2D histogram
f2.SetLineColor(ROOT.kMagenta)
f2.SetLineWidth(4)
f2.Draw("SAME CONT3")

# Save canvas
c.Update()
c.SaveAs("coinc_45_fit.png")


out_file.Close()
print("Saved ROOT file and PNG with moment-based analysis and ellipse overlay.")
