import pyfasterac as pyf
import ROOT
import os
import numpy as np
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt
from scipy.stats import linregress

# Known sources and gamma energies (keV)
sources = {
    "Na-22": [511, 1274],
    "Co-60": [1173, 1332],
    "Cs-137": [662],
}

time_runs = {
    "Na-22": 5*60,
    "Co-60": 5*60,
    "Cs-137": 5*60,
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



#%%


# Prepare data: convert channels to energy using the fitted calibration function
channels_det2 = np.array(channels[2])
maximum_det2 = np.array(maximum[2])

# Convert to energy using the fitted ROOT TF1
calib_func = calibration_old[2]
energies_det2 = np.array(energies[2])

# Log-transform
log_E = np.log(energies_det2)
log_eff = np.log(maximum_det2)

# Linear fit in log-log space
slope, intercept, r_value, p_value, std_err = linregress(log_E, log_eff)

alpha_fit = -slope
norm_fit = np.exp(intercept)

print(f"\n📈 Log-log Fit Results for Detector 2:")
print(f"  Fitted alpha: {alpha_fit:.3f} ± {std_err:.3f}")
print(f"  Normalization (Norm): {norm_fit:.3e}")
print(f"  R² of fit: {r_value**2:.4f}")

# Plot the result
plt.figure(figsize=(8, 6))
plt.scatter(log_E, log_eff, color='blue', label='Data (log-log)', zorder=3)
plt.plot(log_E, slope * log_E + intercept, color='red', label=f'Fit: log(η) = {intercept:.2f} - {alpha_fit:.2f}·log(E)')
plt.xlabel("log(Energy [keV])")
plt.ylabel("log(Relative Efficiency)")
plt.title("Photoelectric Efficiency (log-log fit) - Detector 2")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("photoeff_loglog_fit_detector2.png")
plt.show()




out_file.Close()
print("Calibration saved in calibration.root")
