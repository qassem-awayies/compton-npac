import numpy as np
from scipy.optimize import curve_fit

# Linear function
def linear(x, m, b):
    return m * x + b

# Compute slope, chi2, and predicted DeltaE/E at E_test
def compute_fit(filename, E_test=1000):
    # Load data: x, y, errx, erry
    data = np.loadtxt(filename)
    x, y, errx, erry = data.T

    # Weighted linear fit using erry as sigma
    popt, pcov = curve_fit(linear, x, y, sigma=erry, absolute_sigma=True)
    m, b = popt
    dm, db = np.sqrt(np.diag(pcov))

    # Compute chi-squared
    y_fit = linear(x, m, b)
    chi2 = np.sum(((y - y_fit) / erry) ** 2)
    dof = len(x) - 2

    # Convert E_test to x
    x_test = 1 / np.sqrt(E_test)
    # Predicted DeltaE/E and uncertainty
    y_test = linear(x_test, m, b)
    y_test_err = np.sqrt((x_test * dm) ** 2 + db ** 2)

    print(f"{filename}:")
    print(f"  slope = {m:.4f} ± {dm:.4f}")
    print(f"  intercept = {b:.4f} ± {db:.4f}")
    print(f"  chi2 = {chi2:.2f}, chi2/dof = {chi2/dof:.2f}")
    print(f"  DeltaE/E at E={E_test} = {y_test:.4f} ± {y_test_err:.4f}\n")

# Run for both files
compute_fit('resDet1.dat', E_test=1000)
compute_fit('resDet2.dat', E_test=1000)

