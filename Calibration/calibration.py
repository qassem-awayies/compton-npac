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

# Errors
err_q = 1414.21  # channel error
err_E = 0.1      # energy error

# Paths to your .fast files
file_template = "./{source}.fast/{source}_0001.fast"

# Output ROOT file
out_file = ROOT.TFile("calibration.root", "RECREATE")

# Helper function: extract centroid from histogram
def get_peak_centroid(hist, peak_guess, window=20):
    bin_guess = hist.FindBin(peak_guess)
    start = max(1, bin_guess - window)
    end = min(hist.GetNbinsX(), bin_guess + window)
    max_bin = int(start + np.argmax([hist.GetBinContent(i) for i in range(start, end + 1)]))
    centroid = hist.GetBinCenter(max_bin)
    return centroid

# Prepare per-detector data containers
channels = {1: [], 2: []}
energies = {1: [], 2: []}
labels = {1: [], 2: []}  # <-- for labeling points

# Determine max_q across all files
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

# Number of bins for histograms
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

    hist_det = {}
    for det in [1, 2]:
        hist_det[det] = ROOT.TH1D(f"hist_{source}_det{det}",
                                  f"{source} Detector {det} channel histogram",
                                  nbins, 0, max_q)

    while reader.get_next_event():
        event = reader.get_event()
        for sub_event in event.sub_events:
            det_id = sub_event.label % 1000
            if det_id in [1, 2]:
                hist_det[det_id].Fill(sub_event.q)

    for det in [1, 2]:
        for E_gamma in gammas:
            q_guess = calibration_old[det](E_gamma)
            centroid_q = get_peak_centroid(hist_det[det], peak_guess=q_guess)
            print(f"  Detector {det} Gamma {E_gamma} keV → channel {centroid_q:.2f}")
            channels[det].append(centroid_q)
            energies[det].append(E_gamma)
            labels[det].append(f"{source} {E_gamma} keV")  # <-- label added

        hist_det[det].SetDirectory(out_file)
        hist_det[det].Write()

# Fit and plot per detector
calibration_tf1 = {}
for det in [1, 2]:
    n_points = len(channels[det])
    graph = ROOT.TGraphErrors(
        n_points,
        np.array(channels[det], dtype=np.float64),
        np.array(energies[det], dtype=np.float64),
        np.full(n_points, err_q, dtype=np.float64),
        np.full(n_points, err_E, dtype=np.float64)
    )
    graph.SetName(f"graph_cal_det{det}")
    graph.SetTitle("")
    graph.SetMarkerStyle(20)
    graph.SetMarkerColor(ROOT.kBlue)
    graph.SetLineColor(ROOT.kRed)
    graph.SetLineWidth(2)
    graph.SetMarkerSize(1.0)
    graph.Write()

    f_lin = ROOT.TF1(f"calibration_det{det}", "[0]*x + [1]", 0, max_q)
    f_lin.SetParameter(0, 0.0018)
    f_lin.SetParameter(1, -30)
    graph.Fit(f_lin, "Q")  # quiet fit
    calibration_tf1[det] = f_lin
    f_lin.Write()

    slope = f_lin.GetParameter(0)
    intercept = f_lin.GetParameter(1)
    sigma_m = f_lin.GetParError(0)
    sigma_b = f_lin.GetParError(1)

    print(f"\nDetector {det} calibration: E(Channel) = {slope:.6f} ± {sigma_m:.6f} * Channel {intercept:+.3f} ± {sigma_b:.3f} keV")

    c = ROOT.TCanvas(f"c_cal_det{det}", f"Calibration Detector {det}", 800, 600)
    graph.Draw("AP")
    f_lin.Draw("same")
    c.SetGrid()

    x_min = min(channels[det]) * 0.95
    x_max = max(channels[det]) * 1.05
    ymin = min(energies[det]) * 0.85
    ymax = max(energies[det]) * 1.20
    graph.GetXaxis().SetRangeUser(x_min, x_max)
    graph.GetYaxis().SetRangeUser(ymin, ymax)
    graph.GetXaxis().SetTitle("Channel")
    graph.GetYaxis().SetTitle("Energy (keV)")

    # Legend
    legend = ROOT.TLegend(0.15, 0.78, 0.40, 0.88)
    legend.SetFillColor(ROOT.kWhite)
    legend.SetFillStyle(1001)
    legend.SetBorderSize(2)
    legend.SetMargin(0.2)
    legend.SetTextSize(0.020)
    legend.SetTextFont(42)
    legend.AddEntry(graph, "Measured calibration points", "P")
    legend.AddEntry(f_lin, "Fitted calibration line", "L")
    legend.Draw()

    c.SaveAs(f"calibration_detector_{det}.pdf")

# ------------------------------------------------------------
# Combined calibration plot (Detector 1 vs Detector 2, black labels close to line)
# ------------------------------------------------------------
print("\nCreating combined calibration plot...")

if all(len(channels[det]) > 0 for det in (1, 2)):
    c_combined = ROOT.TCanvas("c_cal_combined", "Combined Calibration", 900, 650)
    c_combined.SetGrid()
    ROOT.gStyle.SetOptStat(0)
    c_combined.SetTicks(1, 1)

    # Determine global axis limits
    all_x = np.concatenate([channels[1], channels[2]])
    all_y = np.concatenate([energies[1], energies[2]])
    x_min, x_max = min(all_x) * 0.95, max(all_x) * 1.05
    y_min, y_max = min(all_y) * 0.85, max(all_y) * 1.20

    # Detector 1 graph (red)
    g1 = ROOT.TGraphErrors(
        len(channels[1]),
        np.array(channels[1], dtype=np.float64),
        np.array(energies[1], dtype=np.float64),
        np.full(len(channels[1]), err_q, dtype=np.float64),
        np.full(len(channels[1]), err_E, dtype=np.float64)
    )
    g1.SetMarkerStyle(20)
    g1.SetMarkerColor(ROOT.kRed + 1)
    g1.SetLineColor(ROOT.kRed + 1)
    g1.SetLineWidth(2)
    g1.SetTitle("")
    g1.GetXaxis().SetTitle("Channel")
    g1.GetYaxis().SetTitle("Energy (keV)")
    g1.GetXaxis().SetLimits(x_min, x_max)
    g1.GetYaxis().SetRangeUser(y_min, y_max)
    g1.Draw("AP")

    # Detector 2 graph (blue)
    g2 = ROOT.TGraphErrors(
        len(channels[2]),
        np.array(channels[2], dtype=np.float64),
        np.array(energies[2], dtype=np.float64),
        np.full(len(channels[2]), err_q, dtype=np.float64),
        np.full(len(channels[2]), err_E, dtype=np.float64)
    )
    g2.SetMarkerStyle(21)
    g2.SetMarkerColor(ROOT.kBlue + 1)
    g2.SetLineColor(ROOT.kBlue + 1)
    g2.SetLineWidth(2)
    g2.Draw("P SAME")

    # Clone and draw fitted lines
    f1_clone, f2_clone = None, None
    if 1 in calibration_tf1:
        f1_clone = calibration_tf1[1].Clone("f1_clone")
        f1_clone.SetLineColor(ROOT.kRed + 1)
        f1_clone.SetLineWidth(2)
        f1_clone.Draw("SAME")
    if 2 in calibration_tf1:
        f2_clone = calibration_tf1[2].Clone("f2_clone")
        f2_clone.SetLineColor(ROOT.kBlue + 1)
        f2_clone.SetLineWidth(2)
        f2_clone.Draw("SAME")

# --- Labels (Detector 2, bold black with exaggerated white edge) ---
latex = ROOT.TLatex()
latex.SetTextSize(0.020)
latex.SetTextFont(62)  # bold Helvetica

if f2_clone:
    used_y = []
    for xi, yi, label in sorted(zip(channels[2], energies[2], labels[2]), key=lambda t: t[1]):
        y_fit = f2_clone.Eval(xi)
        y_label = y_fit + 20  # slight offset above the fit line
        for y_prev in used_y:
            if abs(y_label - y_prev) < 15:
                y_label = y_prev + 15

        # Draw exaggerated white halo by multiple offsets
        latex.SetTextColor(ROOT.kWhite)
        offsets = [-0.5, 0, 0.5]
        for dx in offsets:
            for dy in offsets:
                if dx == 0 and dy == 0:
                    continue
                latex.DrawLatex(xi + dx, y_label + dy, label)

        # Draw actual text in black on top
        latex.SetTextColor(ROOT.kBlack)
        latex.DrawLatex(xi, y_label, label)

        used_y.append(y_label)

    # --- Legend ---
    legend = ROOT.TLegend(0.12, 0.75, 0.45, 0.88)
    legend.SetBorderSize(1)
    legend.SetFillColor(ROOT.kWhite)
    legend.SetTextSize(0.022)
    legend.AddEntry(g1, "Detector 1 data", "P")
    legend.AddEntry(g2, "Detector 2 data", "P")
    if f1_clone:
        legend.AddEntry(f1_clone, "Detector 1 fit", "L")
    if f2_clone:
        legend.AddEntry(f2_clone, "Detector 2 fit", "L")
    legend.Draw()

    c_combined.Write()
    c_combined.SaveAs("calibration_combined.pdf")
    print("→ Combined calibration saved as calibration_combined.pdf")
else:
    print("Skipping combined plot (missing calibration data for one or both detectors).")



out_file.Close()
print("Calibration saved in calibration.root and plots saved as PDF.")
