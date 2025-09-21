import pyfasterac as pyf
import ROOT
import os
import numpy as np

# --- 1. Paths to FAST files
sources = {
    "Na22": "./Na-22.fast/Na-22_0001.fast",
    "Co60": "./Co-60.fast/Co-60_0001.fast",
    "Cs137": "./Cs-137.fast/Cs-137_0001.fast",
    "Bi207": "./Bi-207.fast/Bi-207_0001.fast"
}

# --- 2. Known gamma energies (keV)
gamma_lines = {
    "Na22": [511, 1275],
    "Co60": [1173, 1332],
    "Cs137": [662],
    "Bi207": [570, 1064, 1770]
}

# --- 3. Open calibration file
cal_file = ROOT.TFile("calibration.root", "READ")
f_cal_det1 = cal_file.Get("calibration_det1")
f_cal_det2 = cal_file.Get("calibration_det2")

if not f_cal_det1 or not f_cal_det2:
    raise RuntimeError("Calibration functions not found in calibration.root")

# Cast to TF1 explicitly
f_cal_det1 = ROOT.TF1(f_cal_det1)
f_cal_det2 = ROOT.TF1(f_cal_det2)

calibration = {
    1: lambda q: f_cal_det1.Eval(q),
    2: lambda q: f_cal_det2.Eval(q)
}


# --- 4. Determine global q range per detector for optimal binning
qmin = {1: float('inf'), 2: float('inf')}
qmax = {1: 0, 2: 0}
for file_path in sources.values():
    if not os.path.exists(file_path):
        continue
    reader = pyf.fastreader(file_path)
    while reader.get_next_event():
        event = reader.get_event()
        for sub_event in event.sub_events:
            det_id = sub_event.label % 1000
            if det_id in [1,2]:
                qmin[det_id] = min(qmin[det_id], sub_event.q)
                qmax[det_id] = max(qmax[det_id], sub_event.q)

print(f"Global q ranges: Detector1: {qmin[1]:.1f}-{qmax[1]:.1f}, Detector2: {qmin[2]:.1f}-{qmax[2]:.1f}")

# --- 5. Output ROOT file
out_file = ROOT.TFile("detector_response_matrices_energy.root", "RECREATE")

# --- 6. Loop over isotopes and detectors
histograms_energy = {1:{}, 2:{}}
response_matrices = {1:{}, 2:{}}

for isotope, file_path in sources.items():
    if not os.path.exists(file_path):
        print(f"Skipping {isotope}, file not found")
        continue

    print(f"Processing {isotope}...")

    for det_id in [1,2]:
        nbins = 2000
        e_min = calibration[det_id](qmin[det_id])
        e_max = calibration[det_id](qmax[det_id])
        hist_e = ROOT.TH1D(f"h_{isotope}_det{det_id}_E",
                           f"{isotope} Detector {det_id} energy;Energy (keV);Counts",
                           nbins, e_min, e_max)

        # Fill energy histogram
        reader = pyf.fastreader(file_path)
        while reader.get_next_event():
            event = reader.get_event()
            for sub_event in event.sub_events:
                if sub_event.label % 1000 == det_id:
                    hist_e.Fill(calibration[det_id](sub_event.q))

        histograms_energy[det_id][isotope] = hist_e
        hist_e.Write()

        # --- Build normalized response matrix
        true_energies = gamma_lines[isotope]
        nbins_x = hist_e.GetNbinsX()
        nbins_y = len(true_energies)
        response_hist = ROOT.TH2D(f"R_{isotope}_det{det_id}",
                                  f"Response {isotope} Detector {det_id};Measured E (keV);True E index",
                                  nbins_x, e_min, e_max,
                                  nbins_y, 0, nbins_y)

        # Fill response: sum counts in ±window keV around true energies
        window = 5  # keV
        for i, E_true in enumerate(true_energies):
            sum_counts = 0
            for j in range(1, nbins_x+1):
                E_bin = hist_e.GetBinCenter(j)
                if abs(E_bin - E_true) <= window:
                    val = hist_e.GetBinContent(j)
                    response_hist.SetBinContent(j, i+1, val)
                    sum_counts += val
            # Normalize row
            if sum_counts > 0:
                for j in range(1, nbins_x+1):
                    old = response_hist.GetBinContent(j, i+1)
                    response_hist.SetBinContent(j, i+1, old / sum_counts)

        response_hist.Write()
        response_matrices[det_id][isotope] = response_hist

out_file.Close()
cal_file.Close()
print("All energy histograms and response matrices saved in detector_response_matrices_energy.root")
