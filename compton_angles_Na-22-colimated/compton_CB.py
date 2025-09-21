import pyfasterac as pyf
import ROOT
import array
import os
import math

# --- Calibration ---
calibration = {
    1: lambda q: 0.001841 * q - 31.41,
    2: lambda q: 0.001628 * q - 16.36
}

# --- Parameters ---
angles = range(0, 181, 15)
file_template = "compton_{angle}_Na-22-colimated.fast/compton_{angle}_Na-22-colimated_0001.fast"
out_dir = "output_cb_proj"
os.makedirs(out_dir, exist_ok=True)

dat_file_path = os.path.join(out_dir, "peak_parameters.dat")
fit_param_file_path = os.path.join(out_dir, "fit_parameters.dat")
box_size = 20

# --- Histogram builder ---
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

# --- Fit 1D projection ---
def fit_1d_projection(hist2d, axis='x', nfit_bins=20):
    if axis == 'x':
        proj = hist2d.ProjectionX()
    else:
        proj = hist2d.ProjectionY()

    max_bin = proj.GetMaximumBin()
    mu_guess = proj.GetXaxis().GetBinCenter(max_bin)
    sigma_guess = proj.GetRMS()

    # 1D Crystal Ball + linear background
    def crystalball_1d_fit(x, par):
        A, mu, sigma, alpha, n, p0, p1 = par
        cb = ROOT.TMath.CrystalBall(x, alpha, n, mu, sigma)
        return float(A*cb + p0 + p1*x)

    xmin = mu_guess - nfit_bins * sigma_guess
    xmax = mu_guess + nfit_bins * sigma_guess
    f1 = ROOT.TF1("f1", crystalball_1d_fit, xmin, xmax, 7)
    f1.SetParameters(proj.GetMaximum(), mu_guess, sigma_guess, 1.5, 3.0, 0, 0)
    f1.FixParameter(3, 1.5)  # alpha fixed
    f1.FixParameter(4, 3.0)  # n fixed

    proj.Fit(f1, "RQ")  # R=range, Q=quiet
    mu = f1.GetParameter(1)
    sigma = f1.GetParameter(2)
    return mu, sigma, f1

# --- 2D Crystal Ball with fixed mu,sigma from projections ---
def crystalball_2d_fixed(xx, par, mu_x, sigma_x, mu_y, sigma_y):
    x, y = xx[0], xx[1]
    A, p0, p1, p2 = par
    cbx = ROOT.TMath.CrystalBall(x, 1.5, 3.0, mu_x, sigma_x)
    cby = ROOT.TMath.CrystalBall(y, 1.5, 3.0, mu_y, sigma_y)
    background = p0 + p1*x + p2*y
    return float(A*cbx*cby + background)

# --- Main loop ---
with open(dat_file_path, "w") as fdat, open(fit_param_file_path, "w") as ffit:
    fdat.write("# angle mu_x mu_y sigma_x sigma_y\n")
    ffit.write("# angle A p0 p1 p2\n")

    for angle in angles:
        file_path = file_template.format(angle=angle)
        if not os.path.exists(file_path):
            print(f"Missing: {file_path}, skipping.")
            continue
        print(f"Processing angle {angle}° ...")
        hist2d = build_histogram(file_path)

        # Fit projections
        mu_x, sigma_x, fit_x = fit_1d_projection(hist2d, 'x')
        mu_y, sigma_y, fit_y = fit_1d_projection(hist2d, 'y')

        # Save 1D fit results
        fdat.write(f"{angle} {mu_x:.2f} {mu_y:.2f} {sigma_x:.2f} {sigma_y:.2f}\n")

        # 2D fit only amplitude + linear background
        x_min = hist2d.GetXaxis().GetXmin()
        x_max = hist2d.GetXaxis().GetXmax()
        y_min = hist2d.GetYaxis().GetXmin()
        y_max = hist2d.GetYaxis().GetXmax()

        def f2_wrapper(xx, par):
            return crystalball_2d_fixed(xx, par, mu_x, sigma_x, mu_y, sigma_y)

        f2 = ROOT.TF2("f2", f2_wrapper, x_min, x_max, y_min, y_max, 4)
        f2.SetParameters(hist2d.GetMaximum(), 0, 0, 0)  # A, p0, p1, p2

        hist2d.Fit(f2, "RSQ")

        params = [f2.GetParameter(i) for i in range(4)]
        ffit.write(f"{angle} " + " ".join(f"{p:.4f}" for p in params) + "\n")

        # Plot
        c = ROOT.TCanvas(f"c_{angle}", f"CrystalBall Fit {angle} deg", 800, 600)
        hist2d.Draw("COLZ")
        f2.SetLineColor(ROOT.kRed)
        f2.SetLineWidth(3)
        f2.Draw("SAME CONT3")
        c.SaveAs(os.path.join(out_dir, f"coinc_{angle}_cb_proj.png"))
        print(f"Done angle {angle}° → PNG + params saved")
