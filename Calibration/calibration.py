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

        hist_det[det].SetDirectory(out_file)
        hist_det[det].Write()

# Fit and plot
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
    graph.SetTitle("")  # no main title
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

    # Canvas
    c = ROOT.TCanvas(f"c_cal_det{det}", f"Calibration Detector {det}", 800, 600)
    graph.Draw("AP")
    f_lin.Draw("same")
    c.SetGrid()

    # Add X-axis and Y-axis margins
    x_min = min(channels[det]) * 0.95
    x_max = max(channels[det]) * 1.05
    graph.GetXaxis().SetRangeUser(x_min, x_max)
    ymin = min(energies[det]) * 0.85
    ymax = max(energies[det]) * 1.20
    graph.GetYaxis().SetRangeUser(ymin, ymax)

    graph.GetXaxis().SetTitle("Channel")
    graph.GetYaxis().SetTitle("Energy (keV)")

    # Labels stacked above the fit line
    latex = ROOT.TLatex()
    latex.SetTextSize(0.022)
    latex.SetTextFont(42)
    latex.SetTextColor(ROOT.kBlack)

    points = []
    idx = 0
    for source, gammas in sources.items():
        for E_gamma in gammas:
            points.append((channels[det][idx], E_gamma, f"{source} {E_gamma} keV"))
            idx += 1
    points.sort(key=lambda x: x[1])
    
    stacked_y = []
    for q_val, E_gamma, label in points:
        y_fit = f_lin.Eval(q_val)
        y_label = y_fit + 20  # base offset above fit
        for sy in stacked_y:
            if abs(y_label - sy) < 10:
                y_label += 10
        latex.DrawLatex(q_val, y_label, label)
        stacked_y.append(y_label)

    # Legend for fit & points (taller, padded, no shadow)
    legend = ROOT.TLegend(0.15, 0.78, 0.40, 0.88)
    legend.SetFillColor(ROOT.kWhite)
    legend.SetFillStyle(1001)   # solid fill, no shadow
    legend.SetBorderSize(2)
    legend.SetMargin(0.2)       # padding inside
    legend.SetTextSize(0.020)
    legend.SetTextFont(42)
    legend.SetShadowColor(0)
    legend.SetEntrySeparation(0.008)  # spacing between lines
    legend.AddEntry(graph, "Measured calibration points", "P")
    legend.AddEntry(f_lin, "Fitted calibration line", "L")
    legend.Draw()

    # Save PDF
    c.SaveAs(f"calibration_detector_{det}.pdf")

out_file.Close()
print("Calibration saved in calibration.root and plots saved as PDF.")

