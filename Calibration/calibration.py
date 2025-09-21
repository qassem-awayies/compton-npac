import pyfasterac as pyf
import ROOT
import os
import numpy as np

# Known sources and gamma energies (keV)
sources = {
    "Na-22": [511, 1274],
    "Co-60": [1173, 1332],
    "Cs-137": [662],
    "Bi-207": [570, 1064]
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
    return centroid

# Prepare per-detector data containers
channels = {1: [], 2: []}
energies = {1: [], 2: []}

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
            centroid_q = get_peak_centroid(hist_det[det], peak_guess=q_guess)
            print(f"  Detector {det} Gamma {E_gamma} keV → channel {centroid_q:.2f}")
            channels[det].append(centroid_q)
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

out_file.Close()
print("Calibration saved in calibration.root")
