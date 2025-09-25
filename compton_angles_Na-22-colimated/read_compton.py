#!/usr/bin/env python3
import ROOT
import os
import math
import numpy as np
from iminuit import Minuit
import matplotlib.pyplot as plt

# ------------------------
# Parameters
# ------------------------
angles = range(0, 181, 15)
out_dir = "../output"
histogram_data_file = os.path.join(out_dir, "histogram_data.root")
results_file_path = os.path.join(out_dir, "fit_results_debug.dat")

# Fit region constraint: how many sigma around the guess center to include
FIT_REGION_BOX_SIZE = 2.5
DEBUG_MODE = True  # Enable detailed debugging
TEST_SINGLE_GAUSSIAN = True  # Try single Gaussian first

# ------------------------
# Functions
# ------------------------

def single_gaussian_2d(x, y, A, mu_x, sigma_x, mu_y, sigma_y, rho):
    """Single 2D Gaussian with correlation"""
    dx = (x - mu_x) / sigma_x
    dy = (y - mu_y) / sigma_y
    exponent = -0.5 / (1 - rho**2) * (dx**2 + dy**2 - 2 * rho * dx * dy)
    return A * math.exp(exponent)

def single_gaussian_plus_plane(xy, par):
    """Single Gaussian + linear background"""
    x, y = xy
    A, mu_x, sigma_x, mu_y, sigma_y, rho = par[0:6]
    B, C, D = par[6:9]
    
    gauss = single_gaussian_2d(x, y, A, mu_x, sigma_x, mu_y, sigma_y, rho)
    return gauss + B + C*x + D*y

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

def chi2_single_gaussian(pars, bin_values):
    """Chi-squared for single Gaussian model"""
    chi2 = 0.0
    for x, y, h in bin_values:
        f = single_gaussian_plus_plane((x, y), pars)
        if h > 0:
            chi2 += (h - f)**2 / h
        else:
            chi2 += f**2 * 1e-6
    return chi2

def chi2_double_gaussian(pars, bin_values):
    """Chi-squared for double Gaussian model"""
    chi2 = 0.0
    for x, y, h in bin_values:
        f = double_tilted_gaussian_plus_plane((x, y), pars)
        if h > 0:
            chi2 += (h - f)**2 / h
        else:
            chi2 += f**2 * 1e-6
    return chi2

# Global variables for iminuit compatibility
bin_values_global = []
model_type_global = "single"

def chi2_single_named(A, mu_x, sigma_x, mu_y, sigma_y, rho, B, C, D):
    pars = [A, mu_x, sigma_x, mu_y, sigma_y, rho, B, C, D]
    return chi2_single_gaussian(pars, bin_values_global)

def chi2_double_named(A1, mu_x1, sigma_x1, mu_y1, sigma_y1, rho1,
                      A2, mu_x2, sigma_x2, mu_y2, sigma_y2, rho2,
                      B, C, D):
    pars = [A1, mu_x1, sigma_x1, mu_y1, sigma_y1, rho1,
            A2, mu_x2, sigma_x2, mu_y2, sigma_y2, rho2,
            B, C, D]
    return chi2_double_gaussian(pars, bin_values_global)

def plot_data_and_guess(bin_values, init_params, angle, model_type="single"):
    """Plot the data and initial guess for debugging"""
    if not DEBUG_MODE:
        return
        
    try:
        x_vals = [x for x, y, h in bin_values]
        y_vals = [y for x, y, h in bin_values]
        h_vals = [h for x, y, h in bin_values]
        
        plt.figure(figsize=(12, 4))
        
        # Plot 1: Data
        plt.subplot(1, 3, 1)
        plt.scatter(x_vals, y_vals, c=h_vals, s=20, cmap='viridis')
        plt.colorbar(label='Counts')
        plt.xlabel('X (keV)')
        plt.ylabel('Y (keV)')
        plt.title(f'Data - Angle {angle}°')
        
        # Plot 2: Initial guess
        plt.subplot(1, 3, 2)
        x_range = np.linspace(min(x_vals), max(x_vals), 50)
        y_range = np.linspace(min(y_vals), max(y_vals), 50)
        X, Y = np.meshgrid(x_range, y_range)
        
        if model_type == "single":
            Z = np.array([[single_gaussian_plus_plane((x, y), init_params) 
                          for x in x_range] for y in y_range])
        else:
            Z = np.array([[double_tilted_gaussian_plus_plane((x, y), init_params) 
                          for x in x_range] for y in y_range])
        
        plt.contour(X, Y, Z, levels=10)
        plt.scatter(x_vals, y_vals, c=h_vals, s=20, cmap='viridis', alpha=0.5)
        plt.xlabel('X (keV)')
        plt.ylabel('Y (keV)')
        plt.title(f'Initial Guess - {model_type}')
        
        # Plot 3: Residuals of initial guess
        plt.subplot(1, 3, 3)
        if model_type == "single":
            predicted = [single_gaussian_plus_plane((x, y), init_params) for x, y, h in bin_values]
        else:
            predicted = [double_tilted_gaussian_plus_plane((x, y), init_params) for x, y, h in bin_values]
        
        residuals = [(h - p) for (x, y, h), p in zip(bin_values, predicted)]
        plt.scatter(x_vals, y_vals, c=residuals, s=20, cmap='RdBu_r')
        plt.colorbar(label='Residual')
        plt.xlabel('X (keV)')
        plt.ylabel('Y (keV)')
        plt.title('Initial Residuals')
        
        plt.tight_layout()
        plt.savefig(f'debug_angle_{angle}_initial.png', dpi=150, bbox_inches='tight')
        plt.close()
        
        print(f"    Debug plot saved: debug_angle_{angle}_initial.png")
        
    except Exception as e:
        print(f"    Warning: Could not create debug plot: {e}")

def fit_single_gaussian(init_params, bin_values):
    """Fit with single 2D Gaussian + plane background"""
    global bin_values_global
    bin_values_global = bin_values
    
    try:
        print(f"    Trying single Gaussian fit...")
        print(f"    Initial: A={init_params[0]:.1f}, mu=({init_params[1]:.1f},{init_params[3]:.1f})")
        
        m = Minuit(chi2_single_named,
                   A=init_params[0],
                   mu_x=init_params[1],
                   sigma_x=init_params[2],
                   mu_y=init_params[3],
                   sigma_y=init_params[4],
                   rho=init_params[5],
                   B=init_params[6],
                   C=init_params[7],
                   D=init_params[8])
        
        m.errordef = 1
        
        # Conservative limits
        m.limits["A"] = (0.1, None)
        m.limits["sigma_x"] = (1.0, 100.0)
        m.limits["sigma_y"] = (1.0, 100.0)
        m.limits["rho"] = (-0.9, 0.9)
        
        # Set position limits based on data
        if len(bin_values) > 0:
            x_vals = [x for x, y, h in bin_values]
            y_vals = [y for x, y, h in bin_values]
            x_range = max(x_vals) - min(x_vals)
            y_range = max(y_vals) - min(y_vals)
            
            m.limits["mu_x"] = (min(x_vals) - x_range*0.2, max(x_vals) + x_range*0.2)
            m.limits["mu_y"] = (min(y_vals) - y_range*0.2, max(y_vals) + y_range*0.2)
        
        m.strategy = 2
        
        # Try fitting
        m.migrad()
        
        success = m.fmin.is_valid
        if not success:
            print(f"      Single Gaussian fit failed: {m.fmin}")
            return None, None, None, None, None, "FAILED"
        
        # Try error calculation
        try:
            m.hesse()
            errors_valid = True
        except:
            errors_valid = False
            print(f"      Error calculation failed")
        
        fitted_params = [m.values[name] for name in ["A", "mu_x", "sigma_x", "mu_y", "sigma_y", "rho", "B", "C", "D"]]
        
        if errors_valid:
            errors = [m.errors[name] for name in ["A", "mu_x", "sigma_x", "mu_y", "sigma_y", "rho", "B", "C", "D"]]
        else:
            errors = [float('nan')] * 9
        
        chi2_val = m.fval
        ndf = len(bin_values) - 9
        reduced_chi2 = chi2_val / ndf if ndf > 0 else float("nan")
        
        print(f"      Single Gaussian: χ²/ndf = {reduced_chi2:.3f}")
        return fitted_params, errors, reduced_chi2, chi2_val, ndf, "SUCCESS"
        
    except Exception as e:
        print(f"      Single Gaussian fit crashed: {e}")
        return None, None, None, None, None, "CRASHED"
    finally:
        bin_values_global = []

def fit_double_gaussian(init_params, bin_values):
    """Fit with double 2D Gaussian + plane background"""
    global bin_values_global
    bin_values_global = bin_values
    
    try:
        print(f"    Trying double Gaussian fit...")
        
        m = Minuit(chi2_double_named,
                   A1=init_params[0], mu_x1=init_params[1], sigma_x1=init_params[2],
                   mu_y1=init_params[3], sigma_y1=init_params[4], rho1=init_params[5],
                   A2=init_params[6], mu_x2=init_params[7], sigma_x2=init_params[8],
                   mu_y2=init_params[9], sigma_y2=init_params[10], rho2=init_params[11],
                   B=init_params[12], C=init_params[13], D=init_params[14])
        
        m.errordef = 1
        
        # Conservative limits
        for name in ["A1", "A2"]:
            m.limits[name] = (0.1, None)
        for name in ["sigma_x1", "sigma_y1", "sigma_x2", "sigma_y2"]:
            m.limits[name] = (1.0, 100.0)
        for name in ["rho1", "rho2"]:
            m.limits[name] = (-0.9, 0.9)
        
        # Set position limits based on data
        if len(bin_values) > 0:
            x_vals = [x for x, y, h in bin_values]
            y_vals = [y for x, y, h in bin_values]
            x_range = max(x_vals) - min(x_vals)
            y_range = max(y_vals) - min(y_vals)
            
            for name in ["mu_x1", "mu_x2"]:
                m.limits[name] = (min(x_vals) - x_range*0.2, max(x_vals) + x_range*0.2)
            for name in ["mu_y1", "mu_y2"]:
                m.limits[name] = (min(y_vals) - y_range*0.2, max(y_vals) + y_range*0.2)
        
        m.strategy = 2
        
        # Try fitting
        m.migrad()
        
        success = m.fmin.is_valid
        if not success:
            print(f"      Double Gaussian fit failed: {m.fmin}")
            return None, None, None, None, None, "FAILED"
        
        # Try error calculation
        try:
            m.hesse()
            errors_valid = True
        except:
            errors_valid = False
            print(f"      Error calculation failed")
        
        fitted_params = [
            m.values["A1"], m.values["mu_x1"], m.values["sigma_x1"], m.values["mu_y1"], m.values["sigma_y1"], m.values["rho1"],
            m.values["A2"], m.values["mu_x2"], m.values["sigma_x2"], m.values["mu_y2"], m.values["sigma_y2"], m.values["rho2"],
            m.values["B"], m.values["C"], m.values["D"]
        ]
        
        if errors_valid:
            errors = [
                m.errors["A1"], m.errors["mu_x1"], m.errors["sigma_x1"], m.errors["mu_y1"], m.errors["sigma_y1"], m.errors["rho1"],
                m.errors["A2"], m.errors["mu_x2"], m.errors["sigma_x2"], m.errors["mu_y2"], m.errors["sigma_y2"], m.errors["rho2"],
                m.errors["B"], m.errors["C"], m.errors["D"]
            ]
        else:
            errors = [float('nan')] * 15
        
        chi2_val = m.fval
        ndf = len(bin_values) - 15
        reduced_chi2 = chi2_val / ndf if ndf > 0 else float("nan")
        
        print(f"      Double Gaussian: χ²/ndf = {reduced_chi2:.3f}")
        return fitted_params, errors, reduced_chi2, chi2_val, ndf, "SUCCESS"
        
    except Exception as e:
        print(f"      Double Gaussian fit crashed: {e}")
        return None, None, None, None, None, "CRASHED"
    finally:
        bin_values_global = []

def load_bin_data_from_tree(tree, metadata=None, box_size_factor=3.0):
    """Load bin data from tree, optionally constraining to a region"""
    bin_values = []
    n_entries = tree.GetEntries()
    
    x_min, x_max = float('-inf'), float('inf')
    y_min, y_max = float('-inf'), float('inf')
    
    if metadata:
        center_x = metadata['mu_x_guess']
        center_y = metadata['mu_y_guess']
        sigma_x = metadata['sigma_x_guess']
        sigma_y = metadata['sigma_y_guess']
        
        x_width = box_size_factor * sigma_x
        y_width = box_size_factor * sigma_y
        
        x_min = center_x - x_width
        x_max = center_x + x_width
        y_min = center_y - y_width
        y_max = center_y + y_width
        
        print(f"    Constraining to: X=[{x_min:.1f},{x_max:.1f}], Y=[{y_min:.1f},{y_max:.1f}]")
    
    for i in range(n_entries):
        tree.GetEntry(i)
        x = tree.x
        y = tree.y
        h = tree.h
        
        if (h > 0 and x_min <= x <= x_max and y_min <= y <= y_max):
            bin_values.append((x, y, h))
    
    return bin_values

def analyze_data_quality(bin_values):
    """Analyze the data quality and suggest fitting approach"""
    if not bin_values:
        return "NO_DATA", {}
    
    x_vals = [x for x, y, h in bin_values]
    y_vals = [y for x, y, h in bin_values]
    h_vals = [h for x, y, h in bin_values]
    
    total_counts = sum(h_vals)
    max_counts = max(h_vals)
    
    # Weighted statistics
    mean_x = sum(x * h for x, y, h in bin_values) / total_counts
    mean_y = sum(y * h for x, y, h in bin_values) / total_counts
    
    var_x = sum(h * (x - mean_x)**2 for x, y, h in bin_values) / total_counts
    var_y = sum(h * (y - mean_y)**2 for x, y, h in bin_values) / total_counts
    
    info = {
        'n_bins': len(bin_values),
        'total_counts': total_counts,
        'max_counts': max_counts,
        'mean_x': mean_x,
        'mean_y': mean_y,
        'std_x': math.sqrt(var_x),
        'std_y': math.sqrt(var_y),
        'x_range': (min(x_vals), max(x_vals)),
        'y_range': (min(y_vals), max(y_vals))
    }
    
    # Assess data quality
    if len(bin_values) < 20:
        quality = "INSUFFICIENT"
    elif max_counts < 10:
        quality = "LOW_STATISTICS"
    elif total_counts < 100:
        quality = "MARGINAL"
    else:
        quality = "GOOD"
    
    return quality, info

# ------------------------
# Main fitting routine
# ------------------------

input_file = ROOT.TFile(histogram_data_file, "READ")
if not input_file or input_file.IsZombie():
    print(f"Error: Could not open {histogram_data_file}")
    exit(1)

metadata_tree = input_file.Get("metadata")
if not metadata_tree:
    print("Error: metadata tree not found")
    input_file.Close()
    exit(1)

# Load metadata
angle_metadata = {}
for i in range(metadata_tree.GetEntries()):
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

print(f"Loaded metadata for {len(angle_metadata)} angles")
print(f"Debug mode: {DEBUG_MODE}")
print(f"Test single Gaussian first: {TEST_SINGLE_GAUSSIAN}")

with open(results_file_path, "w") as f:
    f.write("# angle model mu_x mu_x_err mu_y mu_y_err E_sum E_sum_err deviation chi2 reduced_chi2 status\n")
    
    for angle in angles[:3]:  # Test first 3 angles only
        if angle not in angle_metadata:
            continue
            
        bin_tree = input_file.Get(f"bins_{angle}")
        if not bin_tree:
            continue
            
        print(f"\n=== Fitting angle {angle}° ===")
        
        # Load data with constraint
        metadata = angle_metadata[angle]
        bin_values = load_bin_data_from_tree(bin_tree, metadata, FIT_REGION_BOX_SIZE)
        
        # Analyze data quality
        quality, info = analyze_data_quality(bin_values)
        print(f"  Data quality: {quality}")
        print(f"  {info['n_bins']} bins, {info['total_counts']:.0f} total counts, max={info['max_counts']:.0f}")
        print(f"  Center: ({info['mean_x']:.1f}, {info['mean_y']:.1f})")
        print(f"  Spread: σx={info['std_x']:.1f}, σy={info['std_y']:.1f}")
        
        if quality in ["NO_DATA", "INSUFFICIENT"]:
            print(f"  Skipping due to {quality}")
            continue
        
        # Try single Gaussian first
        if TEST_SINGLE_GAUSSIAN:
            # Single Gaussian initial parameters
            single_init = [
                info['max_counts'] * 0.8,  # A
                info['mean_x'], info['std_x'],  # mu_x, sigma_x
                info['mean_y'], info['std_y'],  # mu_y, sigma_y
                0.0,  # rho
                0.0, 0.0, 0.0  # background plane
            ]
            
            plot_data_and_guess(bin_values, single_init, angle, "single")
            
            single_result = fit_single_gaussian(single_init, bin_values)
            fitted_params_s, errors_s, reduced_chi2_s, chi2_val_s, ndf_s, status_s = single_result
            
            if status_s == "SUCCESS":
                mu_x, mu_x_err = fitted_params_s[1], errors_s[1] if not math.isnan(errors_s[1]) else 0.0
                mu_y, mu_y_err = fitted_params_s[3], errors_s[3] if not math.isnan(errors_s[3]) else 0.0
                E_sum = mu_x + mu_y
                E_sum_err = math.sqrt(mu_x_err**2 + mu_y_err**2) if mu_x_err > 0 and mu_y_err > 0 else 0.0
                deviation = E_sum - 511
                
                f.write(f"{angle} SINGLE {mu_x:.6f} {mu_x_err:.6f} {mu_y:.6f} {mu_y_err:.6f} "
                        f"{E_sum:.6f} {E_sum_err:.6f} {deviation:.6f} {chi2_val_s:.6f} {reduced_chi2_s:.6f} {status_s}\n")
                
                print(f"  ✅ Single Gaussian: E_sum = {E_sum:.2f} ± {E_sum_err:.2f} keV, χ²/ndf = {reduced_chi2_s:.3f}")
                
                # If single Gaussian works well, maybe skip double
                if reduced_chi2_s < 2.0:
                    print(f"  Single Gaussian fit is good, skipping double Gaussian")
                    continue
            else:
                print(f"  ❌ Single Gaussian failed: {status_s}")
        
        # Try double Gaussian if single failed or wasn't good enough
        print(f"  Attempting double Gaussian...")
        
        # Double Gaussian initial parameters
        double_init = [
            info['max_counts'] * 0.4,  # A1
            info['mean_x'] - info['std_x'] * 0.5, max(info['std_x'] * 0.8, 2.0),  # mu_x1, sigma_x1
            info['mean_y'] - info['std_y'] * 0.5, max(info['std_y'] * 0.8, 2.0),  # mu_y1, sigma_y1
            0.0,  # rho1
            
            info['max_counts'] * 0.3,  # A2
            info['mean_x'] + info['std_x'] * 0.5, max(info['std_x'] * 0.8, 2.0),  # mu_x2, sigma_x2
            info['mean_y'] + info['std_y'] * 0.5, max(info['std_y'] * 0.8, 2.0),  # mu_y2, sigma_y2
            0.0,  # rho2
            
            0.0, 0.0, 0.0  # background plane
        ]
        
        plot_data_and_guess(bin_values, double_init, angle, "double")
        
        double_result = fit_double_gaussian(double_init, bin_values)
        fitted_params_d, errors_d, reduced_chi2_d, chi2_val_d, ndf_d, status_d = double_result
        
        if status_d == "SUCCESS":
            mu_x1, mu_x1_err = fitted_params_d[1], errors_d[1] if not math.isnan(errors_d[1]) else 0.0
            mu_y1, mu_y1_err = fitted_params_d[3], errors_d[3] if not math.isnan(errors_d[3]) else 0.0
            E_sum = mu_x1 + mu_y1
            E_sum_err = math.sqrt(mu_x1_err**2 + mu_y1_err**2) if mu_x1_err > 0 and mu_y1_err > 0 else 0.0
            deviation = E_sum - 511
            
            f.write(f"{angle} DOUBLE {mu_x1:.6f} {mu_x1_err:.6f} {mu_y1:.6f} {mu_y1_err:.6f} "
                    f"{E_sum:.6f} {E_sum_err:.6f} {deviation:.6f} {chi2_val_d:.6f} {reduced_chi2_d:.6f} {status_d}\n")
            
            print(f"  ✅ Double Gaussian: E_sum = {E_sum:.2f} ± {E_sum_err:.2f} keV, χ²/ndf = {reduced_chi2_d:.3f}")
        else:
            print(f"  ❌ Double Gaussian failed: {status_d}")

input_file.Close()
print(f"\nDebug results saved to: {results_file_path}")

if DEBUG_MODE:
    print("\nCheck the debug plots (debug_angle_*_initial.png) to see:")
    print("- Whether the constrained region contains the peaks")
    print("- Whether the initial guess is reasonable")
    print("- Whether there are actually two distinct peaks")
