import numpy as np
import matplotlib.pyplot as plt

# Coordinates of the two points
theta_detector = 120 #deg
L = 17.5
photon_energy = 511
electron_mass = 511


theta_detector_rad = theta_detector*np.pi/180
radius = 2.9
xmin, xmax = -2.9, 2.9
y_base = 0


rho_NaI = 3.67
mfp_energy = np.array([1.000E-01,1.500E-01, 2.000E-01, 3.000E-01, 4.000E-01, 5.000E-01, 6.000E-01])
# mfp_values = np.array([1.669E+00, 6.112E-01, 3.285E-01, 1.658E-01, 1.171E-01, 9.497E-02, 8.225E-02]) * rho_NaI #NaI
mfp_values = np.array([3.819E+0, 1.388E+00, 6.982E-01, 2.964E-01, 1.803E-01, 1.313E-01, 1.055E-01 ])* rho_NaI #NaI(Tl)

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


############################################################
############################################################
############################################################
############################################################




def simulation(theta_run):
    
    U = np.random.uniform(0, 1, 10000)
    lambda_cristal = mean_free_path(511)
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




theta_explo = np.radians(np.linspace(0,180,120))

correcting_test = np.array([simulation(t) for t in theta_explo])

#%%####### PLOTTING ########

theta_degrees = np.degrees(theta_explo)

# Plot
plt.figure(figsize=(10, 6))
plt.plot(theta_degrees, correcting_test, label='Transmission Probability', color='blue', linewidth=2)

# Labels and title
plt.xlabel('Scattering Angle θ (degrees)', fontsize=14)
plt.ylabel('Transmission Probability', fontsize=14)
plt.title('Attenuation vs. Scattering Angle', fontsize=16)

# Grid and ticks
plt.grid(True, linestyle='--', alpha=0.6)
plt.xticks(np.arange(0, 190, 30))
# plt.ylim(0, 1.05)
# plt.yscale('log')

# Highlight max and min
max_idx = np.argmax(correcting_test)
min_idx = np.argmin(correcting_test)

plt.plot(theta_degrees[max_idx], correcting_test[max_idx], 'ro', label='Max Transmission')
plt.plot(theta_degrees[min_idx], correcting_test[min_idx], 'go', label='Min Transmission')

plt.legend()
plt.tight_layout()
plt.savefig('Attenuation_profile.png')
plt.show()











