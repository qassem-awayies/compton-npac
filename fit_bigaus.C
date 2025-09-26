#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooGaussian.h"
#include "RooProdPdf.h"
#include "RooArgList.h"
#include "RooFitResult.h"
#include <fstream>
#include <vector>
#include <iostream>

void fit_2D_gauss() {
    TFile* f = TFile::Open("./output/histogram_data.root");
    if(!f || f->IsZombie()) {
        std::cout << "Error opening ROOT file!\n";
        return;
    }

    TTree* metadata = (TTree*)f->Get("metadata");
    if(!metadata) { std::cout << "metadata not found!\n"; return; }

    int angle; double mu_x_guess, mu_y_guess, sigma_x_guess, sigma_y_guess, maximum; int n_bins;
    metadata->SetBranchAddress("angle", &angle);
    metadata->SetBranchAddress("mu_x_guess", &mu_x_guess);
    metadata->SetBranchAddress("mu_y_guess", &mu_y_guess);
    metadata->SetBranchAddress("sigma_x_guess", &sigma_x_guess);
    metadata->SetBranchAddress("sigma_y_guess", &sigma_y_guess);
    metadata->SetBranchAddress("maximum", &maximum);
    metadata->SetBranchAddress("n_bins", &n_bins);

    std::ofstream out("fit_results.dat");
    out << "# angle ";
    out << "muX muX_err sigmaX sigmaX_err muY muY_err sigmaY sigmaY_err\n";

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

        // Define RooFit variables
        RooRealVar X("X","X",0,10000); // adjust range if needed
        RooRealVar Y("Y","Y",0,10000);

        // Create empty RooDataHist
        RooDataHist dataHist("dataHist","dataHist",RooArgList(X,Y));

        int nBins = binTree->GetEntries();
        for(int j=0; j<nBins; ++j) {
            binTree->GetEntry(j);
            for(int k=0; k<(int)h; ++k) { // fill h times
                X = x;
                Y = y;
                dataHist.add(RooArgSet(X,Y));
            }
        }

        // Define 2D Gaussian PDF
        RooRealVar muX("muX","muX",mu_x_guess,0,10000);
        RooRealVar sigmaX("sigmaX","sigmaX",sigma_x_guess,0.01,1000);
        RooGaussian gaussX("gaussX","gaussX",X,muX,sigmaX);

        RooRealVar muY("muY","muY",mu_y_guess,0,10000);
        RooRealVar sigmaY("sigmaY","sigmaY",sigma_y_guess,0.01,1000);
        RooGaussian gaussY("gaussY","gaussY",Y,muY,sigmaY);

        RooProdPdf gauss2D("gauss2D","gauss2D",RooArgList(gaussX,gaussY));

        RooFitResult* result = gauss2D.fitTo(dataHist,RooFit::Save());

        // Write parameters for this angle
        out << angle;
        RooArgList pars = result->floatParsFinal();
        for (int p = 0; p < pars.getSize(); ++p) {
            RooRealVar* var = (RooRealVar*)pars.at(p);
            out << " " << var->getVal() << " " << var->getError();
        }
        out << "\n";

        std::cout << "Angle " << angle << " done.\n";
    }

    out.close();
    f->Close();
    std::cout << "Fit results saved to fit_results.dat\n";
}
