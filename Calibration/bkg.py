import pyfasterac as pyf
import ROOT
import os
import numpy as np


# Paths to your .fast files
file_template = "./Na-22.fast/Na-22_0001.fast"

# Output ROOT file
out_file = ROOT.TFile("bkg.root", "RECREATE")


# Determine max_q across all files for optimal binning
#max_q = 0
file_path = file_template.format()
#if not os.path.exists(file_path):
#    print("Warning:{file_path} does not exist")
#reader_tmp = pyf.fastreader(file_path)
#while reader_tmp.get_next_event():
#    event = reader_tmp.get_event()
#    for sub_event in event.sub_events:
#        det_id = sub_event.label % 1000
#        if det_id in [1, 2]:
#            if sub_event.q > max_q:
#                max_q = sub_event.q

#print(f"Maximum q across all files: {max_q:.1f}")

# Set number of bins for optimal binning
nbins = 500
#bin_width = max_q / nbins
#print(f"Processing background")
reader = pyf.fastreader(file_path)

# Create histograms per detector
hist_det = {}
for det in [1, 2]:
    hist_det[det] = ROOT.TH1D(f"hist_bkg_det{det}",
                                  f"Background Detector {det} channel histogram",
                                  nbins, 0, -1)

# Fill histograms
while reader.get_next_event():
    event = reader.get_event()
    for sub_event in event.sub_events:
        det_id = sub_event.label % 1000
        if det_id in [1, 2]:
             hist_det[det_id].Fill(sub_event.q)


# Save histogram
hist_det[det].SetDirectory(out_file)
hist_det[det].Write()
out_file.Close()
print("Background saved in bkg.root")
