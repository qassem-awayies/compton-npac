#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"
#include <fstream>
#include <iostream>

void plot_histogram() {
    TFile* f = TFile::Open("./output/histogram_data_511.root");
    if(!f || f->IsZombie()) {
        std::cout << "Error opening ROOT file!\n";
        return;
    }

    TTree* metadata = (TTree*)f->Get("metadata");
    if(!metadata) {
        std::cout << "metadata not found!\n";
        return;
    }

    int angle;
    double mu_x_guess, mu_y_guess, sigma_x_guess, sigma_y_guess, maximum;
    int n_bins;

    metadata->SetBranchAddress("angle", &angle);
    metadata->SetBranchAddress("mu_x_guess", &mu_x_guess);
    metadata->SetBranchAddress("mu_y_guess", &mu_y_guess);
    metadata->SetBranchAddress("sigma_x_guess", &sigma_x_guess);
    metadata->SetBranchAddress("sigma_y_guess", &sigma_y_guess);
    metadata->SetBranchAddress("maximum", &maximum);
    metadata->SetBranchAddress("n_bins", &n_bins);

    // Create output ROOT file
    TFile* fout = TFile::Open("histogram_data_511_treated.root", "RECREATE");
    if(!fout || fout->IsZombie()) {
        std::cout << "Error creating output ROOT file!\n";
        return;
    }

    int n_entries = metadata->GetEntries();
    for(int i=0; i<n_entries; ++i) {
        metadata->GetEntry(i);
        std::string tree_name = "bins_" + std::to_string(angle);
        TTree* binTree = (TTree*)f->Get(tree_name.c_str());
        if(!binTree) {
            std::cout << "No tree for angle " << angle << "\n";
            continue;
        }

        double x, y, h;
        binTree->SetBranchAddress("x", &x);
        binTree->SetBranchAddress("y", &y);
        binTree->SetBranchAddress("h", &h);

        // Define crop region around guess values
        double x_min = mu_x_guess - 3*sigma_x_guess;
        double x_max = mu_x_guess + 3*sigma_x_guess;
        double y_min = mu_y_guess - 3*sigma_y_guess;
        double y_max = mu_y_guess + 3*sigma_y_guess;

        // Create cropped histogram
        TH2D* hist = new TH2D(Form("hist_%d", angle),
                              Form("Angle %d;X;Y", angle),
                              100, x_min, x_max,
                              100, y_min, y_max);

        int nBins = binTree->GetEntries();
        for(int j=0; j<nBins; ++j) {
            binTree->GetEntry(j);
            if(x >= x_min && x <= x_max && y >= y_min && y <= y_max) {
                hist->Fill(x, y, h);
            }
        }

        // Write histogram to output file
        fout->cd();
        hist->Write();

        delete hist;
        std::cout << "Processed angle " << angle << "\n";
    }

    fout->Close();
    f->Close();

    std::cout << "Cropped histograms saved to histogram_data_511_treated.root\n";
}
