import os
import math
import numpy as np
from scipy.optimize import minimize
from scipy.integrate import dblquad

#%% DATA IMPORT

def load_fit_results(file_path):
    angles = []
    values_list = []
    errors_list = []

    with open(file_path, "r") as f:
        lines = f.readlines()

    # Skip header (lines starting with '#')
    data_lines = [line for line in lines if not line.strip().startswith("#") and line.strip()]

    for line in data_lines:
        parts = line.strip().split()

        angle = float(parts[0])
        angles.append(angle)

        # Extract alternating value and error
        values = [float(parts[i]) for i in range(1, len(parts), 2)]
        errors = [float(parts[i]) for i in range(2, len(parts), 2)]

        values_list.append(values)
        errors_list.append(errors)

    values_array = np.array(values_list)  # shape: (n_angles, n_parameters)
    errors_array = np.array(errors_list)  # shape: (n_angles, n_parameters)

    return angles, values_array, errors_array



results_file = "../output/fit_results_gaussian.dat"
angles, params, errors = load_fit_results(results_file)


#%% INTEGRATION PROCESS



def propagate_integration_error_mc(cb_func, params, param_errors, n_samples=1000, random_seed=None):

    if random_seed is not None:
        np.random.seed(random_seed)

    params = np.array(params)
    param_errors = np.array(param_errors)

    samples = np.random.normal(loc=params, scale=param_errors, size=(n_samples, len(params)))
    integrals = [integrate_cb2d_grid(cb_func, p) for p in samples]

    return np.mean(integrals), np.std(integrals)



def integrate_cb2d_grid(cb_func, params, nx=100, ny=100):
    A = params[0]
    # print('here')
    mu_x, sigma_x = params[1], params[2]
    mu_y, sigma_y = params[3], params[4]

    x_min, x_max = max(mu_x - 2 * sigma_x, 0), mu_x + 2 * sigma_x
    y_min, y_max = max(mu_y - 2 * sigma_y, 0), mu_y + 2 * sigma_y

    x = np.linspace(x_min, x_max, nx)
    y = np.linspace(y_min, y_max, ny)
    X, Y = np.meshgrid(x, y)

    # Flatten grid and evaluate the function in a vectorized way
    XY = np.stack((X.ravel(), Y.ravel()), axis=1)
    Z = np.array([cb_func((x_, y_), params) for x_, y_ in XY])
    Z = Z.reshape((ny, nx))

    # Integrate using the trapezoidal rule
    dx = (x_max - x_min) / (nx - 1)
    dy = (y_max - y_min) / (ny - 1)
    integral = np.trapz(np.trapz(Z, dx=dx, axis=1), dx=dy)

    return integral





def integrate_cb2d_2sigma_root(cb_func, params):
    # print(cb_func)
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
            # print(cb_func)
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



def tilted_double_gaussian(xy, par):
    """
    2D Tilted Gaussian:
    Input:
        xy: (x, y)
        par: [A, mu_x, sigma_x, mu_y, sigma_y, theta]
    Returns:
        float: value of the 2D rotated Gaussian at (x, y)
    """
    x, y = xy
    A, mu_x, sigma_x, mu_y, sigma_y, theta = par

    # Shift to center
    x_shifted = x - mu_x
    y_shifted = y - mu_y

    # Rotate coordinates
    x_rot = x_shifted * np.cos(theta) + y_shifted * np.sin(theta)
    y_rot = -x_shifted * np.sin(theta) + y_shifted * np.cos(theta)

    # 2D Gaussian in rotated frame
    exponent = -0.5 * ((x_rot / sigma_x)**2 + (y_rot / sigma_y)**2)
    return A * np.exp(exponent)


#%% 


from tqdm import tqdm  # pip install tqdm

all_integrated = []
all_errors = []

positive_indices = [0, 2, 3, 4, 6, 7, 8]

for angle, p, e in tqdm(zip(angles, params, errors), total=len(angles), desc="Integrating"):
    # print(p[positive_indices],e[positive_indices]/100)
    integrated, integrated_err = propagate_integration_error_mc(tilted_double_gaussian, p, e/1000,n_samples=10)
    all_integrated.append(integrated)
    all_errors.append(integrated_err)
    print(f"Angle {angle:.1f}°: Integrated = {integrated:.6f} ± {integrated_err:.6f}")

#%%

run_time = [13500, 3593, 3605, 7200, 3600, 4813, 7026, 3315, 3601, 3819, 45*60, 1*60*60 + 20*60 + 10, 15*60*60 + 38*60 + 30]


def write_fit_results_with_integration(
    file_path,
    angles,
    values_array,
    errors_array,
    integrated_vals,
    integrated_errs
):
    """
    Write a new .dat file with angle, parameters, their errors, and the integration result at the end.
    Each line will contain:
        angle val_1 err_1 val_2 err_2 ... val_16 err_16 integration integration_err
    """

    n_angles = len(angles)
    n_params = values_array.shape[1]

    with open(file_path, "w") as f:
        # Header
        f.write("# angle A A_err mu_x mu_x_err sigma_x sigma_x_err alpha_x alpha_x_err n_x n_x_err mu_y mu_y_err sigma_y sigma_y_err alpha_y alpha_y_err n_y n_y_err B B_err C C_err D D_err E_sum E_sum_err deviation chi2 reduced_chi2 integrated integrated_err run_time\n")

        # Data lines
        for i in range(n_angles):
            angle = angles[i]
            values = values_array[i]
            errors = errors_array[i]
            line = f"{angle:.0f}"  # angle as integer

            for val, err in zip(values, errors):
                line += f" {val:.6f} {err:.6f}"

            # Append integration result
            line += f" {integrated_vals[i]:.6f} {integrated_errs[i]:.6f} {run_time[i]:.6f}\n"
            f.write(line)

    print(f"[Done] File written: {file_path}")


output_path = "C:/Users/theom/OneDrive/Bureau/NPAX/TL/output/fit_results_with_integration.dat"

write_fit_results_with_integration(
    file_path=output_path,
    angles=angles,
    values_array=params,
    errors_array=errors,
    integrated_vals=all_integrated,
    integrated_errs=all_errors
)

















