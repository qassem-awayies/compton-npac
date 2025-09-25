#!/usr/bin/env python3
import ROOT
import os
import math
from iminuit import Minuit

# ------------------------
# Parameters
# ------------------------
angles = range(0, 181, 15)
out_dir = "../output"
histogram_data_file = os.path.join(out_dir, "histogram_data.root")
results_file_path = os.path.join(out_dir, "fit_results_gaus.dat")

# ------------------------
# Core Model
# ------------------------

def tilted_gaussian(x, y, A, mu_x, sigma_x, mu_y, sigma_y, rho):
    dx = (x - mu_x) / sigma_x
    dy = (y - mu_y) / sigma_y
    exponent = -0.5 / (1 - rho**2) * (dx**2 + dy**2 - 2 * rho * dx * dy)
    return A * math.exp(exponent)

def double_gaussian_plane(xy, p):
    x, y = xy
    g1 = tilted_gaussian(x, y, *p[0:6])
    g2 = tilted_gaussian(x, y, *p[6:12])
    return g1 + g2 + p[12] + p[13]*x + p[14]*y + p[15]*x*y

def chi2(pars, bin_vals):
    chi2 = 0.0
    for x, y, h in bin_vals:
        f = double_gaussian_plane((x, y), pars)
        chi2 += (h - f)**2 / h if h > 0 else f**2 * 1e-6
    return chi2

# ------------------------
# Fit Function
# ------------------------

bin_vals_global = []

def chi2_named(
    A1, mu_x1, sigma_x1, mu_y1, sigma_y1, rho1,
    A2, mu_x2, sigma_x2, mu_y2, sigma_y2, rho2,
    B, C, D, E
):
    pars = [
        A1, mu_x1, sigma_x1, mu_y1, sigma_y1, rho1,
        A2, mu_x2, sigma_x2, mu_y2, sigma_y2, rho2,
        B, C, D, E
    ]
    return chi2(pars, bin_vals_global)

def fit(bin_vals, init_params):
    global bin_vals_global
    bin_vals_global = bin_vals

    m = Minuit(chi2_named,
        A1=init_params[0], mu_x1=init_params[1], sigma_x1=init_params[2],
        mu_y1=init_params[3], sigma_y1=init_params[4], rho1=init_params[5],
        A2=init_params[6], mu_x2=init_params[7], sigma_x2=init_params[8],
        mu_y2=init_params[9], sigma_y2=init_params[10], rho2=init_params[11],
        B=init_params[12], C=init_params[13], D=init_params[14], E=init_params[15]
    )

    m.errordef = 1
    m.limits["rho1"] = (-0.95, 0.95)
    m.limits["rho2"] = (-0.95, 0.95)

    m.migrad()
    m.hesse()

    fitted = [m.values[k] for k in m.parameters]
    errors = [m.errors[k] for k in m.parameters]
    chi2_val = m.fval
    ndf = len(bin_vals) - len(fitted)
    rchi2 = chi2_val / ndf if ndf > 0 else float("nan")

    return fitted, errors, chi2_val, rchi2

# ------------------------
# Helpers
# ------------------------

def load_bins(tree):
    return [(tree.x, tree.y, tree.h) for _ in range(tree.GetEntries()) if tree.GetEntry(_) and tree.h > 0]

def guess_init(bin_vals):
    total = sum(h for x, y, h in bin_vals)
    mean_x = sum(x*h for x, y, h in bin_vals) / total
    mean_y = sum(y*h for x, y, h in bin_vals) / total
    var_x = sum(h*(x - mean_x)**2 for x, y, h in bin_vals) / total
    var_y = sum(h*(y - mean_y)**2 for x, y, h in bin_vals) / total
    std_x, std_y = math.sqrt(var_x), math.sqrt(var_y)
    max_h = max(h for x, y, h in bin_vals)

    if std_x > std_y:
        mu_x1, mu_x2 = mean_x - std_x*0.7, mean_x + std_x*0.7
        mu_y1 = mu_y2 = mean_y
    else:
        mu_y1, mu_y2 = mean_y - std_y*0.7, mean_y + std_y*0.7
        mu_x1 = mu_x2 = mean_x

    return [
        max_h*0.4, mu_x1, std_x, mu_y1, std_y, 0.0,
        max_h*0.3, mu_x2, std_x, mu_y2, std_y, 0.0,
        0.0, 0.0, 0.0, 0.0  # B, C, D, E
    ]

# ------------------------
# Main
# ------------------------

input_file = ROOT.TFile(histogram_data_file, "READ")
if not input_file or input_file.IsZombie():
    print("Failed to open input file.")
    exit(1)

with open(results_file_path, "w") as f:
    f.write("# angle mu_x1 mu_y1 mu_x2 mu_y2 chi2 red_chi2 status\n")

    for angle in angles:
        tree = input_file.Get(f"bins_{angle}")
        if not tree:
            continue

        bin_vals = load_bins(tree)
        if len(bin_vals) < 20:
            continue

        init_params = guess_init(bin_vals)
        if init_params is None:
            continue

        try:
            p, e, chi2_val, rchi2 = fit(bin_vals, init_params)
            status = "OK" if rchi2 < 10 else "POOR"
        except:
            p = [float('nan')] * 16
            status = "FAIL"

        f.write(f"{angle} {p[1]:.2f} {p[3]:.2f} {p[7]:.2f} {p[9]:.2f} {chi2_val:.2f} {rchi2:.2f} {status}\n")

input_file.Close()
print(f"Results saved to: {results_file_path}")

