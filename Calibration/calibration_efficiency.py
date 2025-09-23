import pyfasterac as pyf
import ROOT
import os
import numpy as np
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt

# Known sources and gamma energies (keV)
sources = {
    "Na-22": [511, 1274],
    "Co-60": [1173, 1332],
    "Cs-137": [662],
    "Bi-207": [570, 1064]
}

time_runs = {
    "Na-22": 5*60,
    "Co-60": 5*60,
    "Cs-137": 5*60,
    "Bi-207": 5*60 +1
}


# Old calibration equations for channel guesses
calibration_old = {
    1: lambda E: (E + 31.41) / 0.001841,
    2: lambda E: (E + 16.36) / 0.001628
}

# Paths to your .fast files
file_template = "./{source}.fast/{source}_0001.fast"

# Output ROOT file
out_file = ROOT.TFile("calibration.root", "RECREATE")

# Helper function: extract centroid from histogram
def get_peak_centroid(hist, peak_guess, window=20):
    bin_guess = hist.FindBin(peak_guess)
    start = max(1, bin_guess - window)
    end = min(hist.GetNbinsX(), bin_guess + window)
    max_bin = int(start + np.argmax([hist.GetBinContent(i) for i in range(start, end+1)]))
    centroid = hist.GetBinCenter(max_bin)
    peak_counts = hist.GetBinContent(max_bin)
    return centroid, peak_counts

# Prepare per-detector data containers
channels = {1: [], 2: []}
energies = {1: [], 2: []}
maximum = {1: [], 2: []}

# Determine max_q across all files for optimal binning
max_q = 0
for source in sources:
    file_path = file_template.format(source=source)
    if not os.path.exists(file_path):
        continue
    reader_tmp = pyf.fastreader(file_path)
    while reader_tmp.get_next_event():
        event = reader_tmp.get_event()
        for sub_event in event.sub_events:
            det_id = sub_event.label % 1000
            if det_id in [1, 2]:
                if sub_event.q > max_q:
                    max_q = sub_event.q

print(f"Maximum q across all files: {max_q:.1f}")

# Set number of bins for optimal binning
nbins = 2000
bin_width = max_q / nbins
print(f"Using {nbins} bins → bin width = {bin_width:.1f}")

# Loop over sources
for source, gammas in sources.items():
    file_path = file_template.format(source=source)
    if not os.path.exists(file_path):
        continue

    print(f"Processing source: {source}")
    reader = pyf.fastreader(file_path)

    # Create histograms per detector
    hist_det = {}
    for det in [1, 2]:
        hist_det[det] = ROOT.TH1D(f"hist_{source}_det{det}",
                                  f"{source} Detector {det} channel histogram",
                                  nbins, 0, max_q)

    # Fill histograms
    while reader.get_next_event():
        event = reader.get_event()
        for sub_event in event.sub_events:
            det_id = sub_event.label % 1000
            if det_id in [1, 2]:
                hist_det[det_id].Fill(sub_event.q)

    # Extract centroids using old calibration for channel guess
    for det in [1, 2]:
        for E_gamma in gammas:
            q_guess = calibration_old[det](E_gamma)
            centroid_q, peak_counts_q = get_peak_centroid(hist_det[det], peak_guess=q_guess)
            print(f"  Detector {det} Gamma {E_gamma} keV → channel {centroid_q:.2f}")
            channels[det].append(centroid_q)
            maximum[det].append(peak_counts_q / time_runs[source])
            energies[det].append(E_gamma)

        # Save histogram
        hist_det[det].SetDirectory(out_file)
        hist_det[det].Write()

# Fit linear calibration per detector using ROOT and plot
calibration_tf1 = {}
for det in [1, 2]:
    n_points = len(channels[det])
    graph = ROOT.TGraph(n_points, np.array(channels[det], dtype=np.float64), np.array(energies[det], dtype=np.float64))
    graph.SetName(f"graph_cal_det{det}")
    graph.SetTitle(f"Detector {det} calibration;Channel q;Energy keV")
    graph.SetMarkerStyle(20)
    graph.SetMarkerColor(ROOT.kBlue)
    graph.SetLineColor(ROOT.kRed)
    graph.SetLineWidth(2)
    graph.SetMarkerSize(1.2)
    graph.Write()

    # Linear function
    f_lin = ROOT.TF1(f"calibration_det{det}", "[0]*x + [1]", 0, max_q)
    f_lin.SetParameter(0, 0.0018)
    f_lin.SetParameter(1, -30)
    graph.Fit(f_lin, "Q")  # Quiet fit
    calibration_tf1[det] = f_lin
    f_lin.Write()
    print(f"\nDetector {det} calibration: E(q) = {f_lin.GetParameter(0):.6f} * q + {f_lin.GetParameter(1):.3f} keV")

    # Create canvas and plot
    c = ROOT.TCanvas(f"c_cal_det{det}", f"Calibration Detector {det}", 800, 600)
    graph.Draw("AP")
    f_lin.Draw("same")
    c.SetGrid()
    c.SaveAs(f"calibration_detector_{det}.png")


#%%


def photo_e_efficiency(E, alpha, Norm):
    return Norm * (E ** (-alpha))

# Prepare data for Detector 2
E_data = np.array(energies[2])
eff_data = np.array(maximum[2])

# Initial guess: alpha = 3, Norm arbitrary
initial_guess = [3.0, 1.0]

# Fit the data
popt, pcov = curve_fit(photo_e_efficiency, E_data, eff_data, p0=initial_guess)

alpha_fit, norm_fit = popt
alpha_err = np.sqrt(np.diag(pcov))[0]

print(f"\n📈 Fitted alpha for Detector 2: {alpha_fit:.3f} ± {alpha_err:.3f}")
print(f"Normalization factor: {norm_fit:.3e}")

# Plot the data and fit
E_fit = np.linspace(min(E_data)*0.9, max(E_data)*1.1, 200)
eff_fit = photo_e_efficiency(E_fit, *popt)


plt.figure(figsize=(8,6))
plt.scatter(E_data, eff_data, color='blue', label="Data (Detector 2)", zorder=3)
plt.plot(E_fit, eff_fit, color='red', label=f"Fit: $\\eta(E) = {norm_fit:.1e} \\cdot E^{{-{alpha_fit:.2f}}}$")
plt.xlabel("Photon Energy (keV)")
plt.ylabel("Efficiency (arb. units)")
plt.title("Photoelectric Efficiency Fit - Detector 2")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("efficiency_fit_detector2.png")
plt.show()





out_file.Close()
print("Calibration saved in calibration.root")
