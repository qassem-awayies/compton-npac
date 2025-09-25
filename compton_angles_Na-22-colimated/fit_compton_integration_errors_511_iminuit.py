import ROOT
from ROOT import Math
import os
import math
import numpy as np
from scipy.optimize import minimize
from scipy.integrate import dblquad
import array
from iminuit import Minuit
from iminuit.util import describe
from functools import partial

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

def integrate_cb2d_2sigma_root(cb_func, params):
    A = params[0]
    mu_x, sigma_x = params[1], params[2]
    mu_y, sigma_y = params[5], params[6]

    # Integration bounds: ±2σ around mean
    x_min, x_max = max(mu_x - 2 * sigma_x, 0), mu_x + 2 * sigma_x
    y_min, y_max = max(mu_y - 2 * sigma_y, 0), mu_y + 2 * sigma_y

    def integrand(y, x):
        dx = (x - mu_x) / sigma_x
        dy = (y - mu_y) / sigma_y
        if dx**2 + dy**2 <= 4.0:
            return cb_func((x, y), params)
        else:
            return 0.0

    result, error = dblquad(
        integrand,
        x_min, x_max,
        lambda x: y_min,
        lambda x: y_max,
        epsabs=1e-4,
        epsrel=1e-4
    )

    return result

def propagate_integration_error_mc(cb_func, params, param_errors, n_samples=1000, random_seed=None):
    """
    Estimate the propagated uncertainty in a 2D integral using Monte Carlo sampling.

    Parameters:
        cb_func: Callable
            The function to integrate. Must accept the same parameter list format as 'params'.
        params: list or np.ndarray
            The central (best-fit) values of the parameters.
        param_errors: list or np.ndarray
            The standard deviation (1-sigma error) associated with each parameter.
        n_samples: int
            Number of Monte Carlo samples to generate.
        random_seed: int or None
            Optional random seed for reproducibility.

    Returns:
        mean_integral: float
            The mean value of the computed integrals.
        std_integral: float
            The standard deviation (1-sigma uncertainty) of the integrals.
    """
    if random_seed is not None:
        np.random.seed(random_seed)

    params = np.array(params)
    param_errors = np.array(param_errors)

    # Indices of parameters that must remain strictly positive
    positive_indices = [0, 2, 3, 4, 6, 7, 8]  # A, sigma_x, alpha_x, n_x, sigma_y, alpha_y, n_y

    integrals = []
    rejected = 0
    max_attempts = 10 * n_samples  # Avoid infinite loops

    attempts = 0
    while len(integrals) < n_samples and attempts < max_attempts:
        attempts += 1
        sampled_params = np.random.normal(loc=params, scale=param_errors)

        # Check for unphysical values
        if any(sampled_params[i] <= 0 for i in positive_indices):
            rejected += 1
            continue

        try:
            result = integrate_cb2d_2sigma_root(cb_func, sampled_params)
            if np.isfinite(result):
                integrals.append(result)
            else:
                rejected += 1
        except Exception:
            rejected += 1

    if len(integrals) == 0:
        raise RuntimeError("All Monte Carlo samples were rejected due to invalid parameter values.")

    mean_integral = np.mean(integrals)
    std_integral = np.std(integrals)

    if rejected > 0:
        print(f"[Warning] Rejected {rejected} invalid samples out of {attempts} attempts.")

    return mean_integral, std_integral



def chi2_cb2d_named(A, mu_x, sigma_x, alpha_x, n_x,
                         mu_y, sigma_y, alpha_y, n_y,
                         B, C, D, bin_values=None):
    pars = np.array([A, mu_x, sigma_x, alpha_x, n_x,
            mu_y, sigma_y, alpha_y, n_y,
            B, C, D])
    return chi2_to_minimize(pars, bin_values)




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

def fit_with_iminuit(init_params, bin_values):
    param_names = ["A", "mu_x", "sigma_x", "alpha_x", "n_x",
                   "mu_y", "sigma_y", "alpha_y", "n_y",
                   "B", "C", "D"]
    init_dict = dict(zip(param_names, init_params))

    # Freeze bin_values using partial
    chi2_func = partial(chi2_cb2d_named, bin_values=bin_values)

    # Pass the fixed-function without bin_values as a free parameter
    m = Minuit(chi2_func, **init_dict)
    m.errordef = Minuit.LEAST_SQUARES

    m.migrad()
    m.hesse()

    if not m.fmin.is_valid:
        print("Warning: Fit did not converge.")

    fitted_params = [m.values[name] for name in param_names]
    errors = [m.errors[name] for name in param_names]
    chi2_val = m.fval
    ndf = len(bin_values) - len(fitted_params)
    reduced_chi2 = chi2_val / ndf if ndf > 0 else float('nan')

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
    f.write("# angle A A_err mu_x mu_x_err sigma_x sigma_x_err mu_y mu_y_err sigma_y sigma_y_err "
            "E_sum E_sum_err deviation chi2 reduced_chi2 integrated integrated_err\n")

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
        bin_values = np.array(load_bin_data_from_tree(bin_tree))
        metadata = angle_metadata[angle]

        print(f"  Loaded {len(bin_values)} bins (expected {metadata['n_bins']})")

        init_params = np.array([metadata['maximum'],
                       metadata['mu_x_guess'], metadata['sigma_x_guess'], 1.5, 3.0,
                       metadata['mu_y_guess'], metadata['sigma_y_guess'], 1.5, 3.0,
                       0.0, 0.0, 0.0])
        
        fitted_params, errors, reduced_chi2, chi2_val, ndf = fit_with_iminuit(init_params, bin_values)

        
        amplitude, amplitude_err = fitted_params[0], errors[0]
        mu_x, mu_x_err = fitted_params[1], errors[1]
        sigma_x, sigma_x_err = fitted_params[2], errors[2]
        mu_y, mu_y_err = fitted_params[5], errors[5]
        sigma_y, sigma_y_err = fitted_params[6], errors[6]

        E_sum = mu_x + mu_y
        E_sum_err = math.sqrt(mu_x_err**2 + mu_y_err**2)
        deviation = E_sum - 511

        
        integrated_cb_raw, integrated_cb_raw_err = propagate_integration_error_mc(cb2d_integration_ready, fitted_params, errors)
        integrated_cb = integrated_cb_raw/ run_time[angle_index]
        integrated_cb_err = integrated_cb_raw_err / run_time[angle_index]

        f.write(f"{angle} {amplitude:.2f} {amplitude_err:.2f} {mu_x:.2f} {mu_x_err:.2f} {sigma_x:.2f} {sigma_x_err:.2f} "
                f"{mu_y:.2f} {mu_y_err:.2f} {sigma_y:.2f} {sigma_y_err:.2f} "
                f"{E_sum:.2f} {E_sum_err:.2f} {deviation:.2f} "
                f"{chi2_val:.2f} {reduced_chi2:.2f} {integrated_cb:.6f} {integrated_cb_err:.6f}\n")

        print(f"Angle {angle}° done. E1+E2={E_sum:.2f}±{E_sum_err:.2f} keV, deviation={deviation:.2f} keV, "
              f"chi2={chi2_val:.2f}, reduced chi2={reduced_chi2:.2f}")
        print(f"  mu_x={mu_x:.2f}±{mu_x_err:.2f}, sigma_x={sigma_x:.2f}±{sigma_x_err:.2f}")
        print(f"  mu_y={mu_y:.2f}±{mu_y_err:.2f}, sigma_y={sigma_y:.2f}±{sigma_y_err:.2f}")
        print(f"  Integrated yield: {integrated_cb:.6f} ± {integrated_cb_err:.6f} [normalized]")

input_file.Close()
print(f"Fit results saved to {results_file_path}")

