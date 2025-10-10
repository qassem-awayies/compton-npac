#include "TFile.h"
#include "TH2.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooGaussian.h"
#include "RooProdPdf.h"
#include "RooAddPdf.h"
<<<<<<< HEAD
=======
#include "RooExtendPdf.h"
>>>>>>> c0719a6885019842c5967292a9040a1a372a7536
#include "RooGenericPdf.h"
#include "RooFitResult.h"
#include <fstream>
#include <iostream>

// ============================================================================
//  2D Gaussian + bilinear background fit
// ============================================================================
void fit_2D_gauss() {
<<<<<<< HEAD
    TFile* f = TFile::Open("histogram_data_511_treated.root");
    if (!f || f->IsZombie()) {
        std::cout << "Error: cannot open histogram_data_511_treated.root" << std::endl;
        return;
    }

    std::ofstream out("fit_results_511.dat");
    out << "# angle muX1 muX1_err sigmaX1 sigmaX1_err muY1 muY1_err sigmaY1 sigmaY1_err "
           "muX2 muX2_err sigmaX2 sigmaX2_err muY2 muY2_err sigmaY2 sigmaY2_err amp1 amp1_err amp2 amp2_err\n";

    std::vector<int> angles = {0, 15, 30, 45, 60, 75, 90, 105, 120, 135, 150, 165};

    for (int angle : angles) {
        std::string hist_name = "hist_" + std::to_string(angle);
        TH2D* h = (TH2D*)f->Get(hist_name.c_str());
        if (!h) {
            std::cout << "Histogram for angle " << angle << " not found!" << std::endl;
            continue;
        }

        std::cout << "Fitting " << hist_name << " ..." << std::endl;

        double xmin = h->GetXaxis()->GetXmin();
        double xmax = h->GetXaxis()->GetXmax();
        double ymin = h->GetYaxis()->GetXmin();
        double ymax = h->GetYaxis()->GetXmax();

        RooRealVar X("X", "E_D1", xmin, xmax);
        RooRealVar Y("Y", "E_D2", ymin, ymax);
        RooDataHist dataHist("dataHist", "dataHist", RooArgList(X, Y), h);

        int binx, biny, binz;
        h->GetMaximumBin(binx, biny, binz);
        double mu_x_guess = h->GetXaxis()->GetBinCenter(binx);
        double mu_y_guess = h->GetYaxis()->GetBinCenter(biny);
        double sigma_x_guess = h->GetRMS(1);
        double sigma_y_guess = h->GetRMS(2);
        double max_val = h->GetMaximum();

        // --- Signal: Double Gaussian ---
        RooRealVar muX1("muX1", "muX1", mu_x_guess, xmin, xmax);
        RooRealVar sigmaX1("sigmaX1", "sigmaX1", sigma_x_guess, 0.01, 200);
        RooGaussian gaussX1("gaussX1", "gaussX1", X, muX1, sigmaX1);

        RooRealVar muY1("muY1", "muY1", mu_y_guess, ymin, ymax);
        RooRealVar sigmaY1("sigmaY1", "sigmaY1", sigma_y_guess, 0.01, 200);
        RooGaussian gaussY1("gaussY1", "gaussY1", Y, muY1, sigmaY1);
        RooProdPdf gauss2D_1("gauss2D_1", "gauss2D_1", RooArgList(gaussX1, gaussY1));

        RooRealVar muX2("muX2", "muX2", mu_x_guess + 10, xmin, xmax);
        RooRealVar sigmaX2("sigmaX2", "sigmaX2", sigma_x_guess * 1.5, 0.01, 300);
        RooGaussian gaussX2("gaussX2", "gaussX2", X, muX2, sigmaX2);

        RooRealVar muY2("muY2", "muY2", mu_y_guess + 10, ymin, ymax);
        RooRealVar sigmaY2("sigmaY2", "sigmaY2", sigma_y_guess * 1.5, 0.01, 300);
        RooGaussian gaussY2("gaussY2", "gaussY2", Y, muY2, sigmaY2);
        RooProdPdf gauss2D_2("gauss2D_2", "gauss2D_2", RooArgList(gaussX2, gaussY2));
=======
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
>>>>>>> c0719a6885019842c5967292a9040a1a372a7536

        RooRealVar amp1("amp1", "amplitude 1", max_val, 0.01, 10 * max_val);
        RooRealVar amp2("amp2", "amplitude 2", 0.1 * max_val, 0.01, 10 * max_val);

        // --- Background: bilinear, positive coefficients ---
        RooRealVar a0("a0", "a0", 1, 0, 10000);
        RooRealVar aX("aX", "aX", 0.01, 0, 10);
        RooRealVar aY("aY", "aY", 0.01, 0, 10);
        RooGenericPdf bkg("bkg", "a0 + aX*X + aY*Y", RooArgList(a0, aX, aY, X, Y));

        RooRealVar bkg_amp("bkg_amp", "background amplitude", 0.2 * max_val, 0.01, 10 * max_val);

        // --- Total model (extended) ---
        RooAddPdf model("model", "signal + background",
                        RooArgList(gauss2D_1, gauss2D_2, bkg),
                        RooArgList(amp1, amp2, bkg_amp));

        RooFitResult* result = model.fitTo(dataHist, RooFit::Save(), RooFit::Extended(true),
                                           RooFit::PrintLevel(-1));

        if (result->status() != 0)
            std::cout << "Warning: fit may not have converged for angle " << angle << std::endl;

        out << angle << " "
            << muX1.getVal() << " " << muX1.getError() << " "
            << sigmaX1.getVal() << " " << sigmaX1.getError() << " "
            << muY1.getVal() << " " << muY1.getError() << " "
            << sigmaY1.getVal() << " " << sigmaY1.getError() << " "
            << muX2.getVal() << " " << muX2.getError() << " "
            << sigmaX2.getVal() << " " << sigmaX2.getError() << " "
            << muY2.getVal() << " " << muY2.getError() << " "
            << sigmaY2.getVal() << " " << sigmaY2.getError() << " "
            << amp1.getVal() << " " << amp1.getError() << " "
            << amp2.getVal() << " " << amp2.getError() << "\n";

        std::cout << "Angle " << angle << " fitted." << std::endl;
    }

    out.close();
    f->Close();
<<<<<<< HEAD
    std::cout << "All fits done. Results saved to fit_results_511.dat" << std::endl;
=======
    std::cout << "Fit results saved to fit_results_1274_bilinear.dat\n";
>>>>>>> c0719a6885019842c5967292a9040a1a372a7536
}
