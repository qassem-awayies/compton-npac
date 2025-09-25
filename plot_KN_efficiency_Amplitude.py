import numpy as np
import os
import matplotlib.pyplot as plt


# Path to the output files
out_dir = "C:\\Users\\theom\\OneDrive\\Bureau\\NPAX\\TL\\output"
dat_file_path = os.path.join(out_dir, "moments_parameters.dat")
fit_file_path = os.path.join(out_dir, "fit_parameters.dat")

# Reading data from moments_parameters.dat
angles_moments = []
integrated_values_moments = []
energy_moments = []

with open(dat_file_path, 'r') as f:
    for line in f:
        if line.strip().startswith('#') or not line.strip():
            continue
        data = line.strip().split()
        angle = float(data[0])
        energy = float(data[2])
        integrated = float(data[7])
        angles_moments.append(angle)
        integrated_values_moments.append(integrated)
        energy_moments.append(energy)
        
angles_moments = np.array(angles_moments)
integrated_values_moments = np.array(integrated_values_moments)
energy_moments = np.array(energy_moments)

# Reading data from fit_parameters.dat
angles_fit = []
integrated_values_fit = []
energy_fits = []
energies_error_fit = []
amplitue_fits = []
time_fits = []

with open(fit_file_path, 'r') as f:
    for line in f:
        if line.strip().startswith('#') or not line.strip():
            continue
        data = line.strip().split()
        angle = float(data[0])
        amplitue_fits.append(float(data[1]))
        energy = float(data[2])
        energies_error_fit.append(float(data[4]))
        integrated = float(data[15])
        time_fits.append(float(data[16]))
        angles_fit.append(angle)
        integrated_values_fit.append(integrated)
        energy_fits.append(energy)


angles_fit = np.array(angles_fit)
integrated_values_fit = np.array(integrated_values_fit)
energy_fits = np.array(energy_fits)
energies_error_fit = np.array(energies_error_fit)
amplitue_fits = np.array(amplitue_fits)
time_fits = np.array(time_fits)
amplitue_fits = amplitue_fits/time_fits

activity = 204200
solid_angle_origin = 0.17
efficiency_norm = np.exp(16.91) / (activity*solid_angle_origin)
alpha = 2.11

def efficiency(E, N=efficiency_norm, alpha = alpha):
    return N / (E**alpha)


#CORRECTING
corrected_integrated = integrated_values_fit/efficiency(energy_fits)
corrected_integrated = corrected_integrated/(corrected_integrated[0])
corrected_amplitude = amplitue_fits/efficiency(energy_fits)
corrected_amplitude = corrected_amplitude/(corrected_amplitude[0])

corrected_energies_error_fit = energies_error_fit/efficiency(energy_fits)
corrected_energies_error_fit = corrected_energies_error_fit/(corrected_integrated[0])
#NON CORRECTED
integrated_values_fit = integrated_values_fit/(integrated_values_fit[0])
energies_error_fit = energies_error_fit/(integrated_values_fit[0])
amplitue_fits = amplitue_fits/(amplitue_fits[0])




###################

def klein_nishina_dsigma_domega(theta_deg, E0_keV=511):
    """
    Compute Klein-Nishina differential cross section for each angle theta (in degrees)
    at incident photon energy E0_keV (default 661.7 keV for Cs-137).
    Returns values in arbitrary units (normalized later).
    """
    # Convert to radians
    theta_rad = np.deg2rad(theta_deg)
    
    # Convert to units of m_e*c^2
    E0 = E0_keV / 511.0

    # Compute scattered energy E'
    E_prime = E0 / (1 + E0 * (1 - np.cos(theta_rad)))

    # Cross section formula
    term1 = E_prime / E0
    term2 = E_prime / E0 + E0 / E_prime - np.sin(theta_rad)**2
    cross_section = 0.5 * term1**2 * term2

    return cross_section


kn_values = klein_nishina_dsigma_domega(angles_fit)  # angles in degrees
kn_values_normalized = kn_values / np.max(kn_values)







#### PLOTTING ########

# Set up figure and axis
fig, ax = plt.subplots(figsize=(10, 6))

# Plot corrected data with error bars
ax.errorbar(
    angles_fit,
    corrected_integrated,
    # yerr=corrected_energies_error_fit,
    xerr=2,
    fmt='o',
    color='blue',
    ecolor='lightblue',
    elinewidth=1.5,
    capsize=3,
    label='Integrated (Klein-Nishina)'
)

# Plot uncorrected data with error bars
ax.errorbar(
    angles_fit,
    corrected_amplitude,
    # yerr=energies_error_fit,
    xerr=2,
    fmt='s',
    color='green',
    ecolor='lightgreen',
    elinewidth=1.5,
    capsize=3,
    label='Amplitude'
)



ax.plot(
    angles_fit,
    kn_values_normalized,
    linestyle='--',
    color='red',
    linewidth=2,
    label='Klein-Nishina Theory (661.7 keV)'
)


# Axis labels and title
ax.set_xlabel("Scattering Angle (degrees)", fontsize=12)
ax.set_ylabel("Normalized Intensity (a.u.)", fontsize=12)
ax.set_title("Klein-Nishina Cross Section - Experimental Data", fontsize=14)

# Grid, legend, and layout
ax.grid(True, linestyle='--', alpha=0.6)
ax.legend()
plt.tight_layout()
plt.show()












