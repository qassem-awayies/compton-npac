import ROOT
import os
import math
import numpy as np
from iminuit import Minuit

# ------------------------
# Parameters
# ------------------------
angles = range(0, 181, 15)
out_dir = "../output"
histogram_data_file = os.path.join(out_dir, "histogram_data.root")
results_file_path = os.path.join(out_dir, "fit_results_gaus.dat")

# ------------------------
# Functions
# ------------------------

def tilted_gaussian(x, y, A, mu_x, sigma_x, mu_y, sigma_y, rho):
    dx = (x - mu_x) / sigma_x
    dy = (y - mu_y) / sigma_y
    exponent = -0.5 / (1 - rho**2) * (dx**2 + dy**2 - 2 * rho * dx * dy)
    return A * math.exp(exponent)

def double_tilted_gaussian_plus_plane(xy, par):
    x, y = xy
    # First Gaussian
    A1, mu_x1, sigma_x1, mu_y1, sigma_y1, rho1 = par[0:6]
    # Second Gaussian
    A2, mu_x2, sigma_x2, mu_y2, sigma_y2, rho2 = par[6:12]
    # Background plane
    B, C, D = par[12:15]

    g1 = tilted_gaussian(x, y, A1, mu_x1, sigma_x1, mu_y1, sigma_y1, rho1)
    g2 = tilted_gaussian(x, y, A2, mu_x2, sigma_x2, mu_y2, sigma_y2, rho2)
    return g1 + g2 + B + C*x + D*y

def chi2_to_minimize(pars, bin_values):
    chi2 = 0.0
    for x, y, h in bin_values:
        f = double_tilted_gaussian_plus_plane((x, y), pars)
        if h > 0:
            chi2 += (h - f)**2 / h
    return chi2

def fit_with_iminuit(init_params, bin_values):
    m = Minuit.from_array_func(
        lambda p: chi2_to_minimize(p, bin_values),
        init_params,
        name=[
            "A1", "mu_x1", "sigma_x1", "mu_y1", "sigma_y1", "rho1",
            "A2", "mu_x2", "sigma_x2", "mu_y2", "sigma_y2", "rho2",
            "B", "C", "D"
        ],
        errordef=1
    )

    # Optional: parameter limits
    for name in ["sigma_x1", "sigma_y1", "sigma_x2", "sigma_y2"]:
        m.limits[name] = (1e-3, None)
    m.limits["rho1"] = (-0.99, 0.99)
    m.limits["rho2"] = (-0.99, 0.99)
    m.limits["A1"] = (0, None)
    m.limits["A2"] = (0, None)

    m.migrad()
    m.hesse()

    if not m.fmin.is_valid:
        print("Warning: Fit did not converge properly.")

    fitted_params = list(m.values.values())
    errors = list(m.errors.values())
    chi2_val = m.fval
    ndf = len(bin_values) - len(fitted_params)
    reduced_chi2 = chi2_val / ndf if ndf > 0 else float("nan")
    return fitted_params, errors, reduced_chi2, chi2_val, ndf

def load_bin_data_from_tree(tree):
    bin_values = []
    n_entries = tree.GetEntries()
    for i in range(n_entries):
        tree.GetEntry(i)
        x = tree.x
        y = tree.y
        h = tree.h
        bin_values.append((x, y, h))
    return bin_values

# ------------------------
# Load histogram data and fit
# ------------------------
input_file = ROOT.TFile(histogram_data_file, "READ")
if not input_file or input_file.IsZombie():
    print(f"Error: Could not open {histogram_data_file}. Please run data_reader.py first.")
    exit(1)

metadata_tree = input_file.Get("metadata")
if not metadata_tree:
    print("Error: metadata tree not found in ROOT file.")
    input_file.Close()
    exit(1)

print(f"Loaded ROOT file with {metadata_tree.GetEntries()} angles")

angle_metadata = {}
n_entries = metadata_tree.GetEntries()
for i in range(n_entries):
    metadata_tree.GetEntry(i)
    angle = metadata_tree.angle
    angle_metadata[angle] = {
        'mu_x_guess': metadata_tree.mu_x_guess,
        'mu_y_guess': metadata_tree.mu_y_guess,
        'sigma_x_guess': metadata_tree.sigma_x_guess,
        'sigma_y_guess': metadata_tree.sigma_y_guess,
        'maximum': metadata_tree.maximum,
        'n_bins': metadata_tree.n_bins
    }

with open(results_file_path, "w") as f:
    f.write("# angle mu_x1 mu_x1_err mu_y1 mu_y1_err mu_x2 mu_x2_err mu_y2 mu_y2_err "
            "E_sum E_sum_err deviation chi2 reduced_chi2\n")

    for angle_index, angle in enumerate(angles):
        if angle not in angle_metadata:
            print(f"No metadata for angle {angle}°, skipping.")
            continue

        bin_tree_name = f"bins_{angle}"
        bin_tree = input_file.Get(bin_tree_name)
        if not bin_tree:
            print(f"No bin data tree found for angle {angle}°, skipping.")
            continue

        print(f"Fitting angle {angle}° ...")
        bin_values = load_bin_data_from_tree(bin_tree)
        metadata = angle_metadata[angle]

        print(f"  Loaded {len(bin_values)} bins (expected {metadata['n_bins']})")

        init_params = [
            metadata['maximum'] * 0.6,  # A1
            metadata['mu_x_guess'] - 2.0, metadata['sigma_x_guess'],  # mu_x1, sigma_x1
            metadata['mu_y_guess'] - 2.0, metadata['sigma_y_guess'],  # mu_y1, sigma_y1
            0.0,  # rho1

            metadata['maximum'] * 0.4,  # A2
            metadata['mu_x_guess'] + 2.0, metadata['sigma_x_guess'],  # mu_x2, sigma_x2
            metadata['mu_y_guess'] + 2.0, metadata['sigma_y_guess'],  # mu_y2, sigma_y2
            0.0,  # rho2

            0.0, 0.0, 0.0  # B, C, D (plane)
        ]

        fitted_params, errors, reduced_chi2, chi2_val, ndf = fit_with_iminuit(init_params, bin_values)

        # Extract relevant values
        mu_x1, mu_x1_err = fitted_params[1], errors[1]
        mu_y1, mu_y1_err = fitted_params[3], errors[3]
        mu_x2, mu_x2_err = fitted_params[7], errors[7]
        mu_y2, mu_y2_err = fitted_params[9], errors[9]

        E_sum = mu_x1 + mu_y1
        E_sum_err = math.sqrt(mu_x1_err**2 + mu_y1_err**2)
        deviation = E_sum - 511

        f.write(f"{angle} {mu_x1:.6f} {mu_x1_err:.6f} {mu_y1:.6f} {mu_y1_err:.6f} "
                f"{mu_x2:.6f} {mu_x2_err:.6f} {mu_y2:.6f} {mu_y2_err:.6f} "
                f"{E_sum:.6f} {E_sum_err:.6f} {deviation:.6f} "
                f"{chi2_val:.6f} {reduced_chi2:.6f}\n")

        print(f"  Done. E1+E2 = {E_sum:.6f} ± {E_sum_err:.6f} keV (Δ = {deviation:.6f} keV)")
        print(f"  Reduced chi2 = {reduced_chi2:.6f}")

input_file.Close()
print(f"\n Fit results saved to: {results_file_path}")

