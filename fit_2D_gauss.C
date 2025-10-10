#include "TFile.h"
#include "TH2.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooGaussian.h"
#include "RooProdPdf.h"
#include "RooAddPdf.h"
#include "RooGenericPdf.h"
#include "RooFitResult.h"
#include <fstream>
#include <iostream>

void fit_2D_gauss() {
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
    std::cout << "All fits done. Results saved to fit_results_511.dat" << std::endl;
}
