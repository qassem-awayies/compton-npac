#include "TFile.h"
#include "TTree.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooGaussian.h"
#include "RooProdPdf.h"
#include "RooAddPdf.h"
#include "RooExtendPdf.h"
#include "RooGenericPdf.h"
#include "RooFitResult.h"
#include <fstream>
#include <iostream>

// ============================================================================
//  2D Gaussian + bilinear background fit
// ============================================================================
void fit_2D_gauss() {
    // Open ROOT file
    TFile* f = TFile::Open("./output/histogram_data.root");
    if(!f || f->IsZombie()) { 
        std::cerr << "Error opening ROOT file!\n";
        return;
    }

    // Metadata tree
    TTree* metadata = (TTree*)f->Get("metadata");
    if(!metadata) { 
        std::cerr << "metadata not found!\n"; 
        return;
    }

    // Metadata variables
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
    out << "# angle muX muX_err sigmaX sigmaX_err muY muY_err sigmaY sigmaY_err "
           "amplitude amplitude_err p0 p1 p2 p3 nbkg\n";

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

        // Determine data bounds for normalization
        double Xmin = 1e9, Xmax = -1e9, Ymin = 1e9, Ymax = -1e9;
        Long64_t nEntries = binTree->GetEntries();
        for (Long64_t j=0; j<nEntries; ++j) {
            binTree->GetEntry(j);
            if (x < Xmin) Xmin = x;
            if (x > Xmax) Xmax = x;
            if (y < Ymin) Ymin = y;
            if (y > Ymax) Ymax = y;
        }

        // RooFit observables
        RooRealVar X("X","X", Xmin, Xmax);
        RooRealVar Y("Y","Y", Ymin, Ymax);

        RooDataHist dataHist("dataHist","dataHist", RooArgList(X,Y));

        // Fill RooDataHist
        for (Long64_t j=0; j<nEntries; ++j) {
            binTree->GetEntry(j);
            for (int k=0; k<(int)h; ++k) {
                X.setVal(x);
                Y.setVal(y);
                dataHist.add(RooArgSet(X,Y));
            }
        }

        // --- 2D Gaussian signal ---
        RooRealVar muX("muX","muX", mu_x_guess, mu_x_guess*0.8, mu_x_guess*1.2);
        RooRealVar sigmaX("sigmaX","sigmaX", sigma_x_guess, 0.01, sigma_x_guess*2);
        RooGaussian gaussX("gaussX","gaussX", X, muX, sigmaX);

        RooRealVar muY("muY","muY", mu_y_guess, mu_y_guess*0.8, mu_y_guess*1.2);
        RooRealVar sigmaY("sigmaY","sigmaY", sigma_y_guess, 0.01, sigma_y_guess*2);
        RooGaussian gaussY("gaussY","gaussY", Y, muY, sigmaY);

        RooProdPdf gauss2D("gauss2D","2D Gaussian", RooArgList(gaussX,gaussY));

        RooRealVar nsig("nsig","signal yield", maximum, 0.0, 10*maximum);
        RooExtendPdf sig_ext("sig_ext","extended Gaussian", gauss2D, nsig);

        // --- Bilinear background: p0 + p1*X + p2*Y + p3*X*Y ---
        double DX = Xmax - Xmin;
        double DY = Ymax - Ymin;
        double A  = DX * DY;
        double IX = 0.5*(Xmax*Xmax - Xmin*Xmin);
        double IY = 0.5*(Ymax*Ymax - Ymin*Ymin);
        double IXY = IX * IY; // factorized integral

        RooRealVar p0("p0","bkg offset", 1e-3, 0.0, 1.0);
        RooRealVar p1("p1","bkg slope X", 0.0, -1.0, 1.0);
        RooRealVar p2("p2","bkg slope Y", 0.0, -1.0, 1.0);
        RooRealVar p3("p3","bkg bilinear", 0.0, -1.0, 1.0);

        RooRealVar z0("z0","Z0", A);
        RooRealVar z1("z1","Z1", IX*DY);
        RooRealVar z2("z2","Z2", DX*IY);
        RooRealVar z3("z3","Z3", IXY);

        // normalized bilinear background
        RooGenericPdf bkg2D("bkg2D",
            "(p0 + p1*X + p2*Y + p3*X*Y) / (z0 + z1*p1 + z2*p2 + z3*p3)",
            RooArgList(p0,p1,p2,p3,X,Y,z0,z1,z2,z3));

        RooRealVar nbkg("nbkg","background yield", 0.05*maximum, 0, 10*maximum);
        RooExtendPdf bkg_ext("bkg_ext","extended background", bkg2D, nbkg);

        // --- Combined model ---
        RooAddPdf model("model","signal+background",
                        RooArgList(sig_ext,bkg_ext));

        RooFitResult* result = nullptr;

        // Special case for angle 165°: cropped region & constraints
        if(angle == 165) {
            double exp_muX = 348.0;
            double exp_muY = 926.0;
            RooDataHist dataHistSubset("dataHistSubset","dataHistSubset",RooArgList(X,Y));

            for (Long64_t j=0; j<nEntries; ++j) {
                binTree->GetEntry(j);
                if(x > exp_muX - 100 && x < exp_muX + 100 &&
                   y > exp_muY - 100 && y < exp_muY + 100){
                    for(int k=0; k<(int)h; ++k){
                        X.setVal(x);
                        Y.setVal(y);
                        dataHistSubset.add(RooArgSet(X,Y));
                    }
                }
            }

            muX.setVal(exp_muX); muX.setRange(exp_muX-30, exp_muX+30);
            muY.setVal(exp_muY); muY.setRange(exp_muY-30, exp_muY+30);
            sigmaX.setVal(20); sigmaX.setRange(5,60);
            sigmaY.setVal(20); sigmaY.setRange(5,60);
            nsig.setVal(maximum); nsig.setRange(0.5*maximum,2*maximum);
            nbkg.setVal(0.05*maximum); nbkg.setRange(0,0.2*maximum);

            result = model.fitTo(dataHistSubset, RooFit::Save(), RooFit::PrintLevel(-1));
        } else {
            result = model.fitTo(dataHist, RooFit::Save(), RooFit::PrintLevel(-1));
        }

        // Save results
        out << angle << " "
            << muX.getVal() << " " << muX.getError() << " "
            << sigmaX.getVal() << " " << sigmaX.getError() << " "
            << muY.getVal() << " " << muY.getError() << " "
            << sigmaY.getVal() << " " << sigmaY.getError() << " "
            << nsig.getVal() << " " << nsig.getError() << " "
            << p0.getVal() << " " << p1.getVal() << " "
            << p2.getVal() << " " << p3.getVal() << " "
            << nbkg.getVal() << "\n";

        std::cout << "Angle " << angle << " done.\n";
    }

    out.close();
    f->Close();
    std::cout << "Fit results saved to fit_results_1274_bilinear.dat\n";
}

