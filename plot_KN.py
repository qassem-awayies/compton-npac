import numpy as np
import matplotlib.pyplot as plt
import os

# Path to the output files
out_dir = "./output"
dat_file_path = os.path.join(out_dir, "moments_parameters.dat")
fit_file_path = os.path.join(out_dir, "fit_parameters.dat")

def read_moments_data(file_path):
    """
    Read the moments_parameters.dat file and extract angle and integrated values.
    """
    angles = []
    integrated_values = []
    
    try:
        with open(file_path, 'r') as f:
            for line in f:
                # Skip comment lines
                if line.strip().startswith('#') or not line.strip():
                    continue
                
                # Parse data line
                # Format: angle mu_x mu_y sigma_x sigma_y E_sum deviation integrated
                data = line.strip().split()
                if len(data) >= 8:
                    angle = float(data[0])
                    integrated = float(data[7])  # integrated value (already divided by time)
                    
                    angles.append(angle)
                    integrated_values.append(integrated)
    
    except FileNotFoundError:
        print(f"Error: File {file_path} not found.")
        return None, None
    
    except Exception as e:
        print(f"Error reading file: {e}")
        return None, None
    
    return np.array(angles), np.array(integrated_values)

def read_fit_data(file_path):
    """
    Read the fit_parameters.dat file and extract angle and integrated values.
    """
    angles = []
    integrated_values = []
    
    try:
        with open(file_path, 'r') as f:
            for line in f:
                # Skip comment lines
                if line.strip().startswith('#') or not line.strip():
                    continue
                
                # Parse data line
                # Format: angle A mu_x sigma_x alpha_x n_x mu_y sigma_y alpha_y n_y B C D chi2 reduced_chi2 integrated
                data = line.strip().split()
                if len(data) >= 16:
                    angle = float(data[0])
                    integrated = float(data[15])  # integrated value (last column)
                    
                    angles.append(angle)
                    integrated_values.append(integrated)
    
    except FileNotFoundError:
        print(f"Error: File {file_path} not found.")
        return None, None
    
    except Exception as e:
        print(f"Error reading file: {e}")
        return None, None
    
    return np.array(angles), np.array(integrated_values)

def plot_integrated_vs_angle(angles, integrated_values, title_suffix="", color='blue', save_path=None):
    """
    Plot integrated counts per unit time vs scattering angle.
    """
    plt.figure(figsize=(10, 7))
    
    # Plot the data points
    colors = {'blue': ('bo-', 'blue', 'darkblue'), 
              'red': ('ro-', 'red', 'darkred'),
              'green': ('go-', 'green', 'darkgreen')}
    
    marker_style, face_color, edge_color = colors.get(color, colors['blue'])
    
    plt.plot(angles, integrated_values, marker_style, markersize=8, linewidth=2, 
             markerfacecolor=face_color, markeredgecolor=edge_color)
    
    # Customize the plot
    plt.xlabel('Scattering Angle (degrees)', fontsize=14)
    plt.ylabel('Integrated Counts / Time (counts/s)', fontsize=14)
    plt.title(f'Compton Scattering: Integrated Count Rate vs Angle{title_suffix}', fontsize=16)
    plt.grid(True, alpha=0.3)
    
    # Set reasonable axis limits
    plt.xlim(-5, 185)
    if len(integrated_values) > 0:
        y_max = np.max(integrated_values) * 1.1
        y_min = max(0, np.min(integrated_values) * 0.9)
        plt.ylim(y_min, y_max)
    
    # Add some styling
    plt.tight_layout()
    
    # Save the plot if path provided
    if save_path:
        plt.savefig(save_path, dpi=300, bbox_inches='tight')
        print(f"Plot saved to: {save_path}")
    
    plt.show()

def plot_comparison(angles1, integrated1, angles2, integrated2, save_path=None):
    """
    Plot both datasets on the same canvas for comparison.
    """
    plt.figure(figsize=(12, 8))
    
    # Plot both datasets
    plt.plot(angles1, integrated1, 'bo-', markersize=8, linewidth=2, 
             markerfacecolor='blue', markeredgecolor='darkblue', 
             label='Moments Analysis', alpha=0.8)
    
    plt.plot(angles2, integrated2, 'ro-', markersize=8, linewidth=2, 
             markerfacecolor='red', markeredgecolor='darkred', 
             label='Crystal Ball Fit', alpha=0.8)
    
    # Customize the plot
    plt.xlabel('Scattering Angle (degrees)', fontsize=14)
    plt.ylabel('Integrated Counts / Time (counts/s)', fontsize=14)
    plt.title('Compton Scattering: Comparison of Integration Methods', fontsize=16)
    plt.grid(True, alpha=0.3)
    plt.legend(fontsize=12)
    
    # Set reasonable axis limits
    plt.xlim(-5, 185)
    all_values = np.concatenate([integrated1, integrated2])
    if len(all_values) > 0:
        y_max = np.max(all_values) * 1.1
        y_min = max(0, np.min(all_values) * 0.9)
        plt.ylim(y_min, y_max)
    
    plt.tight_layout()
    
    # Save the plot if path provided
    if save_path:
        plt.savefig(save_path, dpi=300, bbox_inches='tight')
        print(f"Comparison plot saved to: {save_path}")
    
    plt.show()

def create_log_plots(angles1, integrated1, angles2, integrated2, out_dir):
    """
    Create log-scale plots if there's a large dynamic range.
    """
    all_values = np.concatenate([integrated1, integrated2])
    if len(all_values) > 0 and np.max(all_values) / np.min(all_values) > 10:
        # Individual log plots
        plt.figure(figsize=(10, 7))
        plt.semilogy(angles1, integrated1, 'bo-', markersize=8, linewidth=2,
                     markerfacecolor='blue', markeredgecolor='darkblue', label='Moments Analysis')
        plt.xlabel('Scattering Angle (degrees)', fontsize=14)
        plt.ylabel('Integrated Counts / Time (counts/s)', fontsize=14)
        plt.title('Compton Scattering: Moments Analysis (Log Scale)', fontsize=16)
        plt.grid(True, alpha=0.3)
        plt.xlim(-5, 185)
        plt.tight_layout()
        
        log_save_path1 = os.path.join(out_dir, "integrated_vs_angle_moments_log.png")
        plt.savefig(log_save_path1, dpi=300, bbox_inches='tight')
        print(f"Moments log-scale plot saved to: {log_save_path1}")
        plt.show()
        
        plt.figure(figsize=(10, 7))
        plt.semilogy(angles2, integrated2, 'ro-', markersize=8, linewidth=2,
                     markerfacecolor='red', markeredgecolor='darkred', label='Crystal Ball Fit')
        plt.xlabel('Scattering Angle (degrees)', fontsize=14)
        plt.ylabel('Integrated Counts / Time (counts/s)', fontsize=14)
        plt.title('Compton Scattering: Crystal Ball Fit (Log Scale)', fontsize=16)
        plt.grid(True, alpha=0.3)
        plt.xlim(-5, 185)
        plt.tight_layout()
        
        log_save_path2 = os.path.join(out_dir, "integrated_vs_angle_fit_log.png")
        plt.savefig(log_save_path2, dpi=300, bbox_inches='tight')
        print(f"Fit log-scale plot saved to: {log_save_path2}")
        plt.show()
        
        # Comparison log plot
        plt.figure(figsize=(12, 8))
        plt.semilogy(angles1, integrated1, 'bo-', markersize=8, linewidth=2,
                     markerfacecolor='blue', markeredgecolor='darkblue', 
                     label='Moments Analysis', alpha=0.8)
        plt.semilogy(angles2, integrated2, 'ro-', markersize=8, linewidth=2,
                     markerfacecolor='red', markeredgecolor='darkred', 
                     label='Crystal Ball Fit', alpha=0.8)
        plt.xlabel('Scattering Angle (degrees)', fontsize=14)
        plt.ylabel('Integrated Counts / Time (counts/s)', fontsize=14)
        plt.title('Compton Scattering: Comparison (Log Scale)', fontsize=16)
        plt.grid(True, alpha=0.3)
        plt.legend(fontsize=12)
        plt.xlim(-5, 185)
        plt.tight_layout()
        
        log_save_path_comp = os.path.join(out_dir, "integrated_vs_angle_comparison_log.png")
        plt.savefig(log_save_path_comp, dpi=300, bbox_inches='tight')
        print(f"Comparison log-scale plot saved to: {log_save_path_comp}")
        plt.show()

def analyze_data(angles, integrated_values, dataset_name=""):
    """
    Print some basic statistics about the data.
    """
    if len(angles) == 0:
        print(f"No data to analyze for {dataset_name}.")
        return
    
    print(f"\n{'='*60}")
    print(f"DATA ANALYSIS SUMMARY - {dataset_name.upper()}")
    print(f"{'='*60}")
    print(f"Number of data points: {len(angles)}")
    print(f"Angle range: {np.min(angles):.0f}° to {np.max(angles):.0f}°")
    print(f"Maximum count rate: {np.max(integrated_values):.6f} counts/s at {angles[np.argmax(integrated_values)]:.0f}°")
    print(f"Minimum count rate: {np.min(integrated_values):.6f} counts/s at {angles[np.argmin(integrated_values)]:.0f}°")
    print(f"Average count rate: {np.mean(integrated_values):.6f} counts/s")
    print(f"{'='*60}")

def compare_datasets(angles1, integrated1, angles2, integrated2):
    """
    Compare the two datasets and print statistics.
    """
    # Find common angles
    common_angles = np.intersect1d(angles1, angles2)
    
    if len(common_angles) > 0:
        print(f"\n{'='*60}")
        print("DATASET COMPARISON")
        print(f"{'='*60}")
        print(f"Common angles: {len(common_angles)} out of {len(np.union1d(angles1, angles2))} total")
        
        # Calculate differences for common angles
        diff_values = []
        for angle in common_angles:
            idx1 = np.where(angles1 == angle)[0][0]
            idx2 = np.where(angles2 == angle)[0][0]
            val1 = integrated1[idx1]
            val2 = integrated2[idx2]
            diff = abs(val1 - val2)
            rel_diff = diff / max(val1, val2) * 100
            diff_values.append(rel_diff)
        
        print(f"Average relative difference: {np.mean(diff_values):.2f}%")
        print(f"Maximum relative difference: {np.max(diff_values):.2f}%")
        print(f"{'='*60}")

def main():
    # Read both datasets
    print("Reading Compton scattering data from moments analysis...")
    angles_moments, integrated_moments = read_moments_data(dat_file_path)
    
    print("Reading Compton scattering data from fit analysis...")
    angles_fit, integrated_fit = read_fit_data(fit_file_path)
    
    # Check if we have data
    has_moments = angles_moments is not None and len(angles_moments) > 0
    has_fit = angles_fit is not None and len(angles_fit) > 0
    
    if not has_moments and not has_fit:
        print("No data found in either file. Make sure you have run the Compton analysis script first.")
        return
    
    # Analyze individual datasets
    if has_moments:
        print(f"Successfully read {len(angles_moments)} data points from moments analysis.")
        analyze_data(angles_moments, integrated_moments, "Moments Analysis")
    
    if has_fit:
        print(f"Successfully read {len(angles_fit)} data points from fit analysis.")
        analyze_data(angles_fit, integrated_fit, "Crystal Ball Fit")
    
    # Compare datasets if both are available
    if has_moments and has_fit:
        compare_datasets(angles_moments, integrated_moments, angles_fit, integrated_fit)
    
    # Create individual plots
    if has_moments:
        save_path_moments = os.path.join(out_dir, "integrated_vs_angle_moments.png")
        plot_integrated_vs_angle(angles_moments, integrated_moments, 
                                " (Moments Analysis)", 'blue', save_path_moments)
    
    if has_fit:
        save_path_fit = os.path.join(out_dir, "integrated_vs_angle_fit.png")
        plot_integrated_vs_angle(angles_fit, integrated_fit, 
                                " (Crystal Ball Fit)", 'red', save_path_fit)
    
    # Create comparison plot if both datasets are available
    if has_moments and has_fit:
        save_path_comparison = os.path.join(out_dir, "integrated_vs_angle_comparison.png")
        plot_comparison(angles_moments, integrated_moments, angles_fit, integrated_fit, 
                       save_path_comparison)
    
    # Create log-scale plots if needed
    if has_moments and has_fit:
        create_log_plots(angles_moments, integrated_moments, angles_fit, integrated_fit, out_dir)
    elif has_moments:
        # Just moments data log plot
        if np.max(integrated_moments) / np.min(integrated_moments) > 10:
            plt.figure(figsize=(10, 7))
            plt.semilogy(angles_moments, integrated_moments, 'bo-', markersize=8, linewidth=2,
                         markerfacecolor='blue', markeredgecolor='darkblue')
            plt.xlabel('Scattering Angle (degrees)', fontsize=14)
            plt.ylabel('Integrated Counts / Time (counts/s)', fontsize=14)
            plt.title('Compton Scattering: Moments Analysis (Log Scale)', fontsize=16)
            plt.grid(True, alpha=0.3)
            plt.xlim(-5, 185)
            plt.tight_layout()
            
            log_save_path = os.path.join(out_dir, "integrated_vs_angle_moments_log.png")
            plt.savefig(log_save_path, dpi=300, bbox_inches='tight')
            print(f"Log-scale plot saved to: {log_save_path}")
            plt.show()
    elif has_fit:
        # Just fit data log plot
        if np.max(integrated_fit) / np.min(integrated_fit) > 10:
            plt.figure(figsize=(10, 7))
            plt.semilogy(angles_fit, integrated_fit, 'ro-', markersize=8, linewidth=2,
                         markerfacecolor='red', markeredgecolor='darkred')
            plt.xlabel('Scattering Angle (degrees)', fontsize=14)
            plt.ylabel('Integrated Counts / Time (counts/s)', fontsize=14)
            plt.title('Compton Scattering: Crystal Ball Fit (Log Scale)', fontsize=16)
            plt.grid(True, alpha=0.3)
            plt.xlim(-5, 185)
            plt.tight_layout()
            
            log_save_path = os.path.join(out_dir, "integrated_vs_angle_fit_log.png")
            plt.savefig(log_save_path, dpi=300, bbox_inches='tight')
            print(f"Log-scale plot saved to: {log_save_path}")
            plt.show()

if __name__ == "__main__":
    main()
