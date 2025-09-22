import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1.inset_locator import inset_axes

# --- Compton formula ---
def compton_energy(k, theta_deg):
    theta = np.radians(theta_deg)
    return k / (1 + (k/511.0) * (1 - np.cos(theta)))

def dE_dtheta(k, theta_deg):
    theta = np.radians(theta_deg)
    return (k**2 / 511.0) * np.sin(theta) / (1 + (k/511.0)*(1-np.cos(theta)))**2 * np.pi/180  # keV/deg

# --- Load data ---
angles, E1, sigma_E1, E2, sigma_E2 = [], [], [], [], []
with open("output/fit_parameters.dat") as f:
    for line in f:
        if line.startswith("#"): continue
        cols = line.split()
        if len(cols) < 7: continue
        angle = float(cols[0])
        x0, sigma_x = float(cols[2]), float(cols[3])
        y0, sigma_y = float(cols[4]), float(cols[5])
        angles.append(angle)
        E1.append(x0); sigma_E1.append(sigma_x)
        E2.append(y0); sigma_E2.append(sigma_y)

angles = np.array(angles)
E1 = np.array(E1); sigma_E1 = np.array(sigma_E1)
E2 = np.array(E2); sigma_E2 = np.array(sigma_E2)
Esum = E1 + E2

# --- Angle uncertainty ---
angle_error = 5.0  # degrees
sigma_angle_E1 = dE_dtheta(511, angles) * angle_error
sigma_angle_E2 = dE_dtheta(511, angles) * angle_error
sigma_sum = np.sqrt(sigma_E1**2 + sigma_E2**2 + sigma_angle_E1**2 + sigma_angle_E2**2)

# --- Theoretical curves ---
theta_vals = np.linspace(0, 180, 500)
E1_th = compton_energy(511, theta_vals)
E2_th = 511 - E1_th

# --- Deviation check ---
deviation = 511 - Esum
avg_dev = np.mean(deviation)
print(f"Average deviation (511 - Esum): {avg_dev:.2f} keV")

# --- Main plot: only E1 and E2 ---
fig, ax = plt.subplots(figsize=(8,6))
ax.errorbar(angles, E1, xerr=angle_error, yerr=sigma_E1, fmt='o', color='red', label="E1 measured")
ax.errorbar(angles, E2, xerr=angle_error, yerr=sigma_E2, fmt='o', color='blue', label="E2 measured")
ax.plot(theta_vals, E1_th, 'r--', label="E1 theory")
ax.plot(theta_vals, E2_th, 'b--', label="E2 theory")

ax.set_xlabel("Scattering angle θ (deg)")
ax.set_ylabel("Energy (keV)")
ax.grid(True, linestyle=":")
ax.legend(loc='upper right')

# --- Inset pad: E1+E2, 511, average deviation ---
ax_inset = inset_axes(ax, width="30%", height="30%", loc='lower right')
ax_inset.errorbar(angles, Esum, xerr=angle_error, yerr=sigma_sum, fmt='o', color='green', label="E1+E2")
ax_inset.axhline(511, color='k', linestyle='--', label="511 keV")
ax_inset.set_title(f"Avg dev: {avg_dev:.2f} keV", fontsize=10)
ax_inset.set_xlabel("θ (deg)", fontsize=8)
ax_inset.set_ylabel("Energy (keV)", fontsize=8)
ax_inset.tick_params(axis='both', which='major', labelsize=8)
ax_inset.grid(True, linestyle=":")
ax_inset.legend(fontsize=8)  # keep legend in the inset

plt.tight_layout()
plt.savefig("output/E_vs_theta_with_inset_legend.png", dpi=300)
plt.show()

