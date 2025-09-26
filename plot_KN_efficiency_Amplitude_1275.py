import numpy as np
import os
import matplotlib.pyplot as plt


# Path to the output files
out_dir = ".\output"
dat_file_path = os.path.join(out_dir, "moments_parameters.dat")
fit_file_path = os.path.join(out_dir, "fit_results_with_integration_1274.dat")

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
        integrated = float(data[2])
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
amplitude_error_fit = []
time_fits = []
integration_error_fit = []

with open(fit_file_path, 'r') as f:
    for line in f:
        if line.strip().startswith('#') or not line.strip():
            continue
        data = line.strip().split()
        angle = float(data[0])
        amplitue_fits.append(float(data[1]))
        amplitude_error_fit.append(float(data[2]))
        
        energy = float(data[3])
        energies_error_fit.append(float(data[4]))
        
        integrated = float(data[-3])
        integration_error_fit.append(float(data[-2]))
        
        time_fits.append(float(data[-1]))
        angles_fit.append(angle)
        integrated_values_fit.append(integrated)
        energy_fits.append(energy)


angles_fit = np.array(angles_fit)
integrated_values_fit = np.array(integrated_values_fit)
integration_error_fit = np.array(integration_error_fit)
energy_fits = np.array(energy_fits)
energies_error_fit = np.array(energies_error_fit)
amplitue_fits = np.array(amplitue_fits)
time_fits = np.array(time_fits)
amplitue_fits = amplitue_fits


def resolution(E,ch=2):
    if ch == 2:
        return 0.0006366 + 0.9572 / np.sqrt(E)
    
    if ch == 1:
        return -0.00136 + 1.255 / np.sqrt(E)



amplitude_error_fit_stat = ((np.sqrt(amplitue_fits)) /time_fits)
amplitude_error_fit_stat_syst = ((np.sqrt(amplitue_fits) + amplitue_fits*resolution(energy_fits)) /time_fits) 
amplitue_fits = amplitue_fits/time_fits

activity = 204200
solid_angle_origin = 0.17
efficiency_norm = np.exp(16.91) / (activity*solid_angle_origin)
alpha = 2.11
alpha_err =  0.1 * alpha
N_err =  0.2 * efficiency_norm
calibration_uncertainty = 3.4 / energy_fits + 6e-6/1.8e-3

def efficiency_uncertainty(E, E_err, samples=10000, N=efficiency_norm, N_err = N_err, alpha = alpha, alpha_err = alpha_err):
    # Generate MC samples of alpha and N
    alpha_samples = np.random.normal(alpha, alpha_err, samples)
    N_samples = np.random.normal(efficiency_norm, N_err, samples)
    
    # Compute efficiency for each sample
    efficiencies = N_samples / (E ** alpha_samples)
    
    # Return the mean and std deviation (uncertainty)
    return np.mean(efficiencies), np.std(efficiencies), np.std(efficiencies)/np.mean(efficiencies)

efficiency_energy = np.array([efficiency_uncertainty(E, E*calibration_uncertainty[i])[0] for i,E in enumerate(energy_fits)])
efficiency_energy_err =  np.array([efficiency_uncertainty(E, E*calibration_uncertainty[i])[1] for i,E in enumerate(energy_fits)])
efficiency_err_prop = np.array([efficiency_uncertainty(E, E*calibration_uncertainty[i])[2] for i,E in enumerate(energy_fits)])

###################

def klein_nishina_dsigma_domega(theta_deg, E0_keV=1275):
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




###############################################################################
#############################        Attenuation Process        ################################
###############################################################################

# Coordinates of the two points
theta_detector = 120 #deg
L = 17.5
photon_energy = 1275
electron_mass = 511


theta_detector_rad = theta_detector*np.pi/180
radius = 2.9
xmin, xmax = -2.9, 2.9
y_base = 0


rho_NaI = 3.67
mfp_energy = np.array([1.000E-01,1.500E-01, 2.000E-01, 3.000E-01, 4.000E-01, 5.000E-01, 6.000E-01, 8.000E-01, 1.000E+00, 1.022E+00, 1.250E+00, 1.500E+00])
# mfp_values = np.array([1.669E+00, 6.112E-01, 3.285E-01, 1.658E-01, 1.171E-01, 9.497E-02, 8.225E-02]) * rho_NaI #NaI
mfp_values = np.array([3.819E+0, 1.388E+00, 6.982E-01, 2.964E-01, 1.803E-01, 1.313E-01, 1.055E-01, 7.907E-02, 6.544E-02, 6.430E-02,5.548E-02, 4.980E-02 ])* rho_NaI #NaI(Tl)

def mean_free_path(E, mfp_energy = mfp_energy, mfp_values = mfp_values):
    # Dummy approximation — use real data if available
    # NaI has complex behavior: this is just a placeholder
    return np.interp(E, mfp_energy, mfp_values)


def detector_limits(x,theta_val = theta_detector_rad):
    x_1, y_1 = np.cos(theta_val)*L + np.sin(theta_val)*radius, np.sin(theta_val)*L - np.cos(theta_val)*radius
    x_2, y_2 = np.cos(theta_val)*L - np.sin(theta_val)*radius, np.sin(theta_val)*L +  np.cos(theta_val)*radius
    #############################
    dx_min = x_1 - x
    dy_min = y_1
    #############################
    dx_max = x_2 - x
    dy_max = y_2
    
    # Angle in radians
    theta_rad_min = np.arctan(dy_min / dx_min)
    theta_rad_max = np.arctan(dy_max / dx_max)
    for i in range(len(theta_rad_max)):
        if (theta_rad_min[i] > np.pi/4 or theta_rad_max[i] > np.pi/4) &  (theta_rad_min[i] < 3*np.pi/4 or 3*theta_rad_max[i] < 3*np.pi/4): 
            theta_rad_min[i] = np.mod(theta_rad_min[i], np.pi)
            theta_rad_max[i] = np.mod(theta_rad_max[i], np.pi)
    
    return np.linspace(theta_rad_min, theta_rad_max)

def compton(theta,photon = photon_energy):
    return photon_energy / (1 + photon_energy/electron_mass * (1 - np.cos(theta)))
    

def intersection_distance(x, theta, R=2.9):
    sin_theta = np.sin(theta)
    cos_theta = np.cos(theta)
    
    under_root = R**2 - (x**2) * sin_theta**2
    
    t = -x * cos_theta + np.sqrt(under_root)
    return t   # Only forward intersection

def simulate_absorption(travel_length, energy_keV, rng=None):
    
    lambda_ = mean_free_path(energy_keV)  # in cm
    absorption_distance = np.random.exponential(lambda_)

    go_through = absorption_distance < travel_length
    return go_through, absorption_distance


def simulation(theta_run):
    
    U = np.random.uniform(0, 1, 10000)
    lambda_cristal = mean_free_path(1275)
    interaction_lengths = -lambda_cristal * np.log(U)
    interaction_lengths.sort()
    
    x_space = interaction_lengths[interaction_lengths<5.9] + xmin
    x_space_1d = []
    for x_val in x_space:    
        x_space_1d +=  [x_val]*50
    x_space_1d = np.array(x_space_1d)
    
    theta_coverage = detector_limits(x_space,theta_val=theta_run) 
    theta_coverage_1d = theta_coverage.flatten()
    # print(np.degrees(np.std(theta_coverage_1d)))
    
    
    
    travel_distance = intersection_distance(x_space_1d,theta_coverage_1d)
    
    energy_coverage = compton(theta_coverage)
    energy_coverage_1d = energy_coverage.flatten()
    

    onces_through, distance_of_fly = simulate_absorption(travel_distance, energy_coverage_1d)
    
    return np.sum(onces_through)/len(onces_through) #, travel_distance


attenuation_process = np.array([simulation(t) for t in np.radians(angles_fit)])



###############################################################################
#############################        DATA CORRECTION        #####s###########################
###############################################################################

cross_point = 0


kn_values_normalized = kn_values / kn_values[cross_point]
#DOUBLE CORRECTING
double_corrected_integrated = (integrated_values_fit/attenuation_process)/efficiency_energy
double_corrected_amplitude = (amplitue_fits/attenuation_process)/efficiency_energy



double_corrected_integration_error_fit = (integration_error_fit/attenuation_process)/efficiency_energy
double_corrected_integration_error_fit = double_corrected_integration_error_fit/(double_corrected_integrated[cross_point])
double_corrected_integrated = double_corrected_integrated/(double_corrected_integrated[cross_point])


double_corrected_amplitude_error_fit_stat = (amplitude_error_fit_stat/attenuation_process)/efficiency_energy
double_corrected_amplitude_error_fit_stat_syst = (amplitude_error_fit_stat_syst/attenuation_process)/efficiency_energy



# double_corrected_amplitude_error_fit_stat = np.sqrt(  (amplitude_error_fit_stat*amplitue_fits/efficiency_energy)**2 + (amplitue_fits/(efficiency_energy**2))**2 *  efficiency_energy_err**2          )


# double_corrected_amplitude_error_fit_stat = amplitude_error_fit_stat

double_corrected_amplitude_error_fit_stat = double_corrected_amplitude_error_fit_stat/(double_corrected_amplitude[cross_point])
double_corrected_amplitude_error_fit_stat_syst = double_corrected_amplitude_error_fit_stat_syst/(double_corrected_amplitude[cross_point])
double_corrected_amplitude = double_corrected_amplitude/(double_corrected_amplitude[cross_point])



#CORRECTING
corrected_integrated = (integrated_values_fit)/efficiency_energy
corrected_integrated = corrected_integrated/(corrected_integrated[cross_point])
corrected_amplitude = (amplitue_fits)/efficiency_energy
# corrected_amplitude = corrected_amplitude/(corrected_amplitude[cross_point])




corrected_integration_error_fit = integration_error_fit/efficiency_energy
corrected_integration_error_fit = corrected_integration_error_fit/(corrected_integrated[cross_point])



corrected_amplitude_error_fit = (amplitude_error_fit)/efficiency_energy
corrected_amplitude_error_fit = corrected_amplitude_error_fit/(corrected_amplitude[cross_point])
corrected_amplitude = corrected_amplitude/(corrected_amplitude[cross_point])



#NON CORRECTED
integrated_values_fit = integrated_values_fit/(integrated_values_fit[cross_point])
energies_error_fit = energies_error_fit/(integrated_values_fit[cross_point])
amplitue_fits = amplitue_fits/(amplitue_fits[cross_point])




###############################################################################
#############################        PLOTTING        ################################
###############################################################################
# Set up figure and axis
# fig, ax = plt.subplots(figsize=(10, 6))

# # Plot corrected data with error bars
# ax.errorbar(
#     angles_fit,
#     double_corrected_integrated,
#     # yerr=double_corrected_integration_error_fit,
#     xerr=2,
#     fmt='o',
#     color='blue',
#     ecolor='lightblue',
#     elinewidth=1.5,
#     capsize=3,
#     label='Efficiency + Absorption'
# )


# # Plot uncorrected data with error bars
# ax.errorbar(
#     angles_fit,
#     corrected_integrated,
#     # yerr=corrected_integration_error_fit,
#     xerr=2,
#     fmt='s',
#     color='green',
#     ecolor='lightgreen',
#     elinewidth=1.5,
#     capsize=3,
#     label='Efficiency'
# )



# ax.plot(
#     angles_fit,
#     kn_values_normalized,
#     linestyle='--',
#     color='red',
#     linewidth=2,
#     label='Klein-Nishina Theory (511 keV)'
# )


# # Axis labels and title
# ax.set_xlabel("Scattering Angle (degrees)", fontsize=12)
# ax.set_ylabel("Normalized Intensity (a.u.)", fontsize=12)
# ax.set_title(r"Klein-Nishina Cross Section - $2\sigma$ Integration", fontsize=14)

# # Grid, legend, and layout
# ax.grid(True, linestyle='--', alpha=0.6)
# ax.legend()
# plt.tight_layout()
# plt.show()




#Amplitude figure

fig, ax = plt.subplots(figsize=(10, 6))

# Plot corrected data with error bars
ax.errorbar(
    angles_fit,
    double_corrected_amplitude,
    yerr=double_corrected_amplitude_error_fit_stat,
    xerr=2,
    fmt='o',
    color='blue',
    ecolor='blue',
    elinewidth=1.5,
    capsize=3,
    label='Statistical'
)


# Plot uncorrected data with error bars
ax.errorbar(
    angles_fit,
    double_corrected_amplitude,
    yerr=double_corrected_amplitude_error_fit_stat_syst,
    xerr=2,
    fmt='s',
    color='green',
    ecolor='green',
    elinewidth=1.5,
    capsize=3,
    label='Statistical + Systematics'
)



ax.plot(
    angles_fit,
    kn_values_normalized,
    linestyle='--',
    color='red',
    linewidth=2,
    label='Klein-Nishina Theory (1275 keV)'
)

plt.ylim([0,1.2])

# Axis labels and title
ax.set_xlabel("Scattering Angle (degrees)", fontsize=12)
ax.set_ylabel("Normalized Intensity (a.u.)", fontsize=12)
ax.set_title("Klein-Nishina Cross Section - Amplitude", fontsize=14)

plt.savefig('Klein_Nishima_Efficiency_corrected_1274.png')

# Grid, legend, and layout
ax.grid(True, linestyle='--', alpha=0.6)
ax.legend()
plt.tight_layout()
plt.show()







