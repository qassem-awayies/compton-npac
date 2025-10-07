#include "TFile.h"
#include "TTree.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooGaussian.h"
#include "RooProdPdf.h"
#include "RooAddPdf.h"
#include "RooExtendPdf.h"
#include "RooFitResult.h"
#include <fstream>
#include <iostream>

void fit_2D_gauss() {
    TFile* f = TFile::Open("./output-2ndRun/histogram_data.root");
    if(!f || f->IsZombie()) { std::cout << "Error opening ROOT file!\n"; return; }

    TTree* metadata = (TTree*)f->Get("metadata");
    if(!metadata) { std::cout << "metadata not found!\n"; return; }

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

    std::ofstream out("fit_results.dat");
    out << "# angle muX muX_err sigmaX sigmaX_err muY muY_err sigmaY sigmaY_err amplitude amplitude_err\n";

    int n_entries = metadata->GetEntries();
    for(int i=0; i<n_entries; ++i) {
        metadata->GetEntry(i);
        std::string tree_name = "bins_" + std::to_string(angle);
        TTree* binTree = (TTree*)f->Get(tree_name.c_str());
        if(!binTree) { std::cout << "No tree for angle " << angle << "\n"; continue; }

        double x, y, h;
        binTree->SetBranchAddress("x", &x);
        binTree->SetBranchAddress("y", &y);
        binTree->SetBranchAddress("h", &h);

        RooRealVar X("X","X",0,10000);
        RooRealVar Y("Y","Y",0,10000);
        RooDataHist dataHist("dataHist","dataHist",RooArgList(X,Y));

        int nBins = binTree->GetEntries();
        for(int j=0; j<nBins; ++j) {
            binTree->GetEntry(j);
            for(int k=0; k<(int)h; ++k) {
                X = x; Y = y;
                dataHist.add(RooArgSet(X,Y));
            }
        }

        // --- 2D Gaussian ---
        RooRealVar muX("muX","muX",mu_x_guess, mu_x_guess*0.8, mu_x_guess*1.2);
        RooRealVar sigmaX("sigmaX","sigmaX",sigma_x_guess, 0.01, sigma_x_guess*2);
        RooGaussian gaussX("gaussX","gaussX",X,muX,sigmaX);

        RooRealVar muY("muY","muY",mu_y_guess, mu_y_guess*0.8, mu_y_guess*1.2);
        RooRealVar sigmaY("sigmaY","sigmaY",sigma_y_guess, 0.01, sigma_y_guess*2);
        RooGaussian gaussY("gaussY","gaussY",Y,muY,sigmaY);

        RooProdPdf gauss2D("gauss2D","gauss2D",RooArgList(gaussX,gaussY));

        RooRealVar amp("amp","Gaussian amplitude",maximum,0,10*maximum);
        RooExtendPdf gauss2D_ext("gauss2D_ext","extended 2D Gaussian",gauss2D,amp);

        // --- Constant background ---
        RooRealVar p0("p0","background constant",0.1,0,1000);
        RooGenericPdf bkg2D("bkg2D","bkg2D","p0",RooArgList(p0));

        RooRealVar bkg_frac("bkg_frac","bkg fraction",0.05,0.0,0.2); // constrain fraction
        RooAddPdf model("model","signal + background",RooArgList(gauss2D_ext,bkg2D),RooArgList(bkg_frac));

        RooFitResult* result = model.fitTo(dataHist,RooFit::Save());

        out << angle << " ";
        out << muX.getVal() << " " << muX.getError() << " "
            << sigmaX.getVal() << " " << sigmaX.getError() << " "
            << muY.getVal() << " " << muY.getError() << " "
            << sigmaY.getVal() << " " << sigmaY.getError() << " "
            << amp.getVal() << " " << amp.getError() << "\n";

        std::cout << "Angle " << angle << " done.\n";
    }

    out.close();
    f->Close();
    std::cout << "Fit results saved to fit_results.dat\n";
}

