import ROOT
from ROOT import Math
import os
import math
import numpy as np
from scipy.optimize import minimize
from scipy.integrate import dblquad
import array

# ------------------------
# Parameters
# ------------------------
run_time = [13500, 3593, 3605, 7200, 3600, 4813, 7026, 3315, 3601, 3819, 45*60, 1*60*60 + 20*60 + 10, 15*60*60 + 38*60 + 30]
run_time_new = []
angles = range(0, 181, 15)
out_dir = "../output"
histogram_data_file = os.path.join(out_dir, "histogram_data.root")
results_file_path = os.path.join(out_dir, "fit_results.dat")

# ------------------------
# Functions
# ------------------------
def cb2d_linear_manual(xy, par):
    xx, yy = xy
    A = par[0]
    mu_x, sigma_x, alpha_x, n_x = par[1:5]
    mu_y, sigma_y, alpha_y, n_y = par[5:9]
    B, C, D = par[9:12]

    cbx = ROOT.Math.crystalball_pdf(xx, alpha_x, n_x, sigma_x, mu_x)
    cby = ROOT.Math.crystalball_pdf(yy, alpha_y, n_y, sigma_y, mu_y)

    return A * cbx * cby + B + C*xx + D*yy

def cb2d_integration_ready(xy, par):
    xx, yy = xy
    A = par[0]
    mu_x, sigma_x, alpha_x, n_x = par[1:5]
    mu_y, sigma_y, alpha_y, n_y = par[5:9]

    cbx = ROOT.Math.crystalball_pdf(xx, alpha_x, n_x, sigma_x, mu_x)
    cby = ROOT.Math.crystalball_pdf(yy, alpha_y, n_y, sigma_y, mu_y)

    return A * cbx * cby


def chi2_to_minimize(pars, bin_values):
    chi2 = 0.0
    for x, y, h in bin_values:
        f = cb2d_linear_manual((x, y), pars)
        if h > 0:
            chi2 += (h - f)**2 / h
    return chi2

def calculate_parameter_errors(fitted_params, bin_values, delta=1e-4):
    n_params = len(fitted_params)
    hessian = np.zeros((n_params, n_params))

    for i in range(n_params):
        for j in range(i, n_params):
            params_pp = fitted_params.copy()
            params_pm = fitted_params.copy()
            params_mp = fitted_params.copy()
            params_mm = fitted_params.copy()

            params_pp[i] += delta
            params_pp[j] += delta
            params_pm[i] += delta
            params_pm[j] -= delta
            params_mp[i] -= delta
            params_mp[j] += delta
            params_mm[i] -= delta
            params_mm[j] -= delta

            chi2_pp = chi2_to_minimize(params_pp, bin_values)
            chi2_pm = chi2_to_minimize(params_pm, bin_values)
            chi2_mp = chi2_to_minimize(params_mp, bin_values)
            chi2_mm = chi2_to_minimize(params_mm, bin_values)

            hessian[i][j] = (chi2_pp - chi2_pm - chi2_mp + chi2_mm) / (4 * delta * delta)
            if i != j:
                hessian[j][i] = hessian[i][j]

    try:
        covariance = np.linalg.inv(hessian)
        errors = np.sqrt(np.diag(abs(covariance)))
        return errors
    except np.linalg.LinAlgError:
        print("Warning: Could not calculate parameter errors (singular Hessian matrix)")
        return np.zeros(n_params)

def fit_with_scipy_minimizer(init_params, bin_values):
    result = minimize(chi2_to_minimize, init_params, args=(bin_values,), method='L-BFGS-B')
    fitted_params = result.x
    chi2_val = chi2_to_minimize(fitted_params, bin_values)
    ndf = len(bin_values) - len(fitted_params)
    reduced_chi2 = chi2_val / ndf if ndf > 0 else float('nan')
    errors = calculate_parameter_errors(fitted_params, bin_values)
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
    f.write("# angle A A_err mu_x mu_x_err sigma_x sigma_x_err alpha_x alpha_x_err n_x n_x_err mu_y mu_y_err sigma_y sigma_y_err alpha_y alpha_y_err n_y n_y_err B B_err C C_err D D_err"
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

        init_params = [metadata['maximum'],
                       metadata['mu_x_guess'], metadata['sigma_x_guess'], 1.5, 3.0,
                       metadata['mu_y_guess'], metadata['sigma_y_guess'], 1.5, 3.0,
                       0.0, 0.0, 0.0]

        fitted_params, errors, reduced_chi2, chi2_val, ndf = fit_with_scipy_minimizer(init_params, bin_values)
        
        amplitude, amplitude_err = fitted_params[0], errors[0]
        mu_x, mu_x_err = fitted_params[1], errors[1]
        sigma_x, sigma_x_err = fitted_params[2], errors[2]
        alpha_x, alpha_x_err = fitted_params[3], errors[3]
        n_x, n_x_err = fitted_params[4], errors[4]
        
        mu_y, mu_y_err = fitted_params[5], errors[5]
        sigma_y, sigma_y_err = fitted_params[6], errors[6]
        alpha_y, alpha_y_err = fitted_params[7], errors[7]
        n_y, n_y_err = fitted_params[8], errors[8]
        
        B, B_err = fitted_params[9], errors[9]
        C, C_err = fitted_params[10], errors[10]
        D, D_err = fitted_params[11], errors[11]



        E_sum = mu_x + mu_y
        E_sum_err = math.sqrt(mu_x_err**2 + mu_y_err**2)
        deviation = E_sum - 511


        f.write(f"{angle} {amplitude:.2f} {amplitude_err:.2f} {mu_x:.2f} {mu_x_err:.2f} {sigma_x:.2f} {sigma_x_err:.2f} "
                f"{alpha_x:.2f} {alpha_x_err:.2f} {n_x:.2f} {n_x_err:.2f}"
                f"{mu_y:.2f} {mu_y_err:.2f} {sigma_y:.2f} {sigma_y_err:.2f} "
                f"{alpha_y:.2f} {alpha_y_err:.2f} {n_y:.2f} {n_y_err:.2f}"
                f"{B:.2f} {B_err:.2f} {C:.2f} {C_err:.2f} {D:.2f} {D_err:.2f}"
                f"{E_sum:.2f} {E_sum_err:.2f} {deviation:.2f} "
                f"{chi2_val:.2f} {reduced_chi2:.2f}\n")

        print(f"Angle {angle}° done. E1+E2={E_sum:.2f}±{E_sum_err:.2f} keV, deviation={deviation:.2f} keV, "
              f"chi2={chi2_val:.2f}, reduced chi2={reduced_chi2:.2f}")
        print(f"  mu_x={mu_x:.2f}±{mu_x_err:.2f}, sigma_x={sigma_x:.2f}±{sigma_x_err:.2f}")
        print(f"  mu_y={mu_y:.2f}±{mu_y_err:.2f}, sigma_y={sigma_y:.2f}±{sigma_y_err:.2f}")
        # print(f"  Integrated yield: {integrated_cb:.6f} ± {integrated_cb_err:.6f} [normalized]")

input_file.Close()
print(f"Fit results saved to {results_file_path}")

