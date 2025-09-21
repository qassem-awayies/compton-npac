import pyfasterac as pyf
import ROOT
from ROOT import Math
import array
import os
import math
import numpy as np
from scipy.optimize import minimize

# ------------------------
# Calibration
# ------------------------
calibration = {
    1: lambda q: 0.001841 * q - 31.41,
    2: lambda q: 0.001628 * q - 16.36
}

# ------------------------
# Parameters
# ------------------------
angles = range(0, 181, 15)
file_template = "compton_{angle}_Na-22-colimated.fast/compton_{angle}_Na-22-colimated_0001.fast"
out_dir = "output_cb2d_scipyfit_overlay"
os.makedirs(out_dir, exist_ok=True)

dat_file_path = os.path.join(out_dir, "peak_parameters.dat")
fit_param_file_path = os.path.join(out_dir, "fit_parameters.dat")
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

def cb2d_linear_manual(xy, par):
    xx, yy = xy
    A = par[0]
    mu_x, sigma_x, alpha_x, n_x = par[1:5]
    mu_y, sigma_y, alpha_y, n_y = par[5:9]
    B, C, D = par[9:12]

    cbx = ROOT.Math.crystalball_pdf(xx, alpha_x, n_x, sigma_x, mu_x)
    cby = ROOT.Math.crystalball_pdf(yy, alpha_y, n_y, sigma_y, mu_y)

    return A * cbx * cby + B + C*xx + D*yy

def chi2_to_minimize(pars, bin_values):
    chi2 = 0.0
    for x, y, h in bin_values:
        f = cb2d_linear_manual((x, y), pars)
        if h > 0:
            chi2 += (h - f)**2 / h
    return chi2

# ------------------------
# Main loop
# ------------------------
with open(dat_file_path,"w") as fdat, open(fit_param_file_path,"w") as ffit:
    fdat.write("# angle mu_x mu_y sigma_x sigma_y E_sum deviation\n")
    ffit.write("# angle A mu_x sigma_x alpha_x n_x mu_y sigma_y alpha_y n_y B C D chi2 reduced_chi2\n")

    for angle in angles:
        file_path = file_template.format(angle=angle)
        if not os.path.exists(file_path):
            print(f"Missing {file_path}, skipping.")
            continue

        print(f"Processing angle {angle}° ...")
        h2 = build_histogram(file_path)
        guess = analyze_peak(h2, box_size)
        if guess is None: continue
        mu_x, mu_y, sigma_x, sigma_y, peak_bin_x, peak_bin_y = guess

        # Fit window
        fit_box = max(5, box_size//2)
        x_min = h2.GetXaxis().GetBinLowEdge(max(1, peak_bin_x - fit_box))
        x_max = h2.GetXaxis().GetBinUpEdge(min(h2.GetNbinsX(), peak_bin_x + fit_box))
        y_min = h2.GetYaxis().GetBinLowEdge(max(1, peak_bin_y - fit_box))
        y_max = h2.GetYaxis().GetBinUpEdge(min(h2.GetNbinsY(), peak_bin_y + fit_box))

        # Flatten histogram bins in fit window, ignore low counts
        bin_values = []
        for ix in range(1, h2.GetNbinsX()+1):
            x_val = h2.GetXaxis().GetBinCenter(ix)
            if x_val < x_min or x_val > x_max: continue
            for iy in range(1, h2.GetNbinsY()+1):
                y_val = h2.GetYaxis().GetBinCenter(iy)
                if y_val < y_min or y_val > y_max: continue
                h_val = h2.GetBinContent(ix, iy)
                if h_val < 5:  # ignore very low counts
                    continue
                bin_values.append( (x_val, y_val, h_val) )

        # Initial parameters
        init_params = [h2.GetMaximum(),
                       mu_x, sigma_x, 1.5, 3.0,
                       mu_y, sigma_y, 1.5, 3.0,
                       0.0, 0.0, 0.0]

        # Minimize χ²
        result = minimize(chi2_to_minimize, init_params, args=(bin_values,), method='L-BFGS-B')
        fitted_params = result.x
        chi2_val = chi2_to_minimize(fitted_params, bin_values)
        ndf = len(bin_values) - len(fitted_params)
        reduced_chi2 = chi2_val / ndf if ndf > 0 else float('nan')

        # Save parameters
        E_sum = mu_x + mu_y
        deviation = E_sum - 511
        fdat.write(f"{angle} {mu_x:.2f} {mu_y:.2f} {sigma_x:.2f} {sigma_y:.2f} {E_sum:.2f} {deviation:.2f}\n")
        ffit.write(f"{angle} " + " ".join(f"{p:.4f}" for p in fitted_params) +
                   f" {chi2_val:.2f} {reduced_chi2:.2f}\n")

        print(f"Angle {angle}° done. E1+E2={E_sum:.2f} keV, deviation={deviation:.2f} keV, "
              f"chi2={chi2_val:.2f}, reduced chi2={reduced_chi2:.2f}")

        # Save histogram and overlay CB surface
        out_root = ROOT.TFile(os.path.join(out_dir, f"coinc_{angle}.root"), "RECREATE")
        h2.SetDirectory(out_root)
        h2.Write()

        npoints = 50
        x_vals = np.linspace(x_min, x_max, npoints)
        y_vals = np.linspace(y_min, y_max, npoints)
        g2 = ROOT.TGraph2D()
        idx = 0
        for xi in x_vals:
            for yi in y_vals:
                g2.SetPoint(idx, xi, yi, cb2d_linear_manual((xi, yi), fitted_params))
                idx += 1

        c = ROOT.TCanvas(f"c_{angle}", "", 800, 600)
        h2.Draw("COLZ")
        g2.SetLineColor(ROOT.kRed)
        g2.SetLineWidth(2)
        g2.Draw("SAME SURF1")
        c.SaveAs(os.path.join(out_dir, f"coinc_{angle}.png"))
        out_root.Close()
