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
    TFile* f = TFile::Open("./output/histogram_data_1274.root");
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

    std::ofstream out("fit_results_1274.dat");
    out << "# angle muX muX_err sigmaX sigmaX_err muY muY_err sigmaY sigmaY_err amplitude amplitude_err\n";

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
        RooGenericPdf bkg2D("bkg2D","bkg2D","p0 + 0*X + 0*Y", RooArgList(p0,X,Y));

        RooRealVar bkg_frac("bkg_frac","bkg fraction",0.05,0.0,0.2);
        RooAddPdf model("model","signal + background",RooArgList(gauss2D_ext,bkg2D),RooArgList(bkg_frac));

        RooFitResult* result = nullptr;

        // =============================================================
        // Special handling for angle 165°: crop data and constrain parameters
        // =============================================================
        if(angle == 165) {
            double exp_muX = 348.0;
            double exp_muY = 926.0;

            // Create a cropped RooDataHist
            RooDataHist dataHistSubset("dataHistSubset","dataHistSubset",RooArgList(X,Y));
            for(int j=0; j<nBins; ++j) {
                binTree->GetEntry(j);
                if(x > exp_muX - 100 && x < exp_muX + 100 &&
                   y > exp_muY - 100 && y < exp_muY + 100){
                    for(int k=0; k<(int)h; ++k){
                        X = x; Y = y;
                        dataHistSubset.add(RooArgSet(X,Y));
                    }
                }
            }

            // Constrain parameters near expected values
            muX.setVal(exp_muX); muX.setRange(exp_muX-30, exp_muX+30);
            muY.setVal(exp_muY); muY.setRange(exp_muY-30, exp_muY+30);
            sigmaX.setVal(20); sigmaX.setRange(5,60);
            sigmaY.setVal(20); sigmaY.setRange(5,60);
            amp.setVal(maximum); amp.setRange(0.5*maximum,2*maximum);

            // Reduce background fraction in cropped region
            bkg_frac.setVal(0.05); bkg_frac.setRange(0.0,0.1);

            // Fit only the cropped histogram
            result = model.fitTo(dataHistSubset, RooFit::Save(), RooFit::PrintLevel(-1));
        } else {
            // Regular fit for other angles
            result = model.fitTo(dataHist, RooFit::Save(), RooFit::PrintLevel(-1));
        }

        out << angle << " "
            << muX.getVal() << " " << muX.getError() << " "
            << sigmaX.getVal() << " " << sigmaX.getError() << " "
            << muY.getVal() << " " << muY.getError() << " "
            << sigmaY.getVal() << " " << sigmaY.getError() << " "
            << amp.getVal() << " " << amp.getError() << "\n";

        std::cout << "Angle " << angle << " done.\n";
    }

    out.close();
    f->Close();
    std::cout << "Fit results saved to fit_results_1274.dat\n";
}
