#include <TFile.h>
#include <TH2.h>
#include <TF2.h>
#include <TKey.h>
#include <TIterator.h>
#include <TROOT.h>
#include <TCanvas.h>
#include <TBox.h>
#include <TString.h>
#include <TFitResultPtr.h>
#include <TMath.h>

#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>

// ---------------------
// Tilted 2D Double Gaussian + linear background
// ---------------------
Double_t TiltedDoubleGauss2D(Double_t *x, Double_t *par) {
    double X = x[0], Y = x[1];
    double x0 = par[2], y0 = par[3];
    double theta = par[8];

    double xrot =  (X - x0)*cos(theta) + (Y - y0)*sin(theta);
    double yrot = -(X - x0)*sin(theta) + (Y - y0)*cos(theta);

    double gaus1 = par[0]*exp(-0.5*(xrot*xrot/(par[4]*par[4]) + yrot*yrot/(par[5]*par[5])));
    double gaus2 = par[1]*exp(-0.5*(xrot*xrot/(par[6]*par[6]) + yrot*yrot/(par[7]*par[7])));

    double bg = par[9] + par[10]*X + par[11]*Y + par[12]*X*Y;

    return gaus1 + gaus2 + bg;
}

// ---------------------
// Compute weighted centroid over a box around max bin
// ---------------------
void WeightedCentroid(TH2 *h2, int binxMax, int binyMax, int box_size,
                      double &xCenter, double &yCenter, double &xRMS, double &yRMS) {
    int nbinsX = h2->GetXaxis()->GetNbins();
    int nbinsY = h2->GetYaxis()->GetNbins();

    double sum_w = 0.0, sum_x = 0.0, sum_y = 0.0;
    double sum_x2 = 0.0, sum_y2 = 0.0;

    for (int i = std::max(1, binxMax-box_size); i <= std::min(nbinsX, binxMax+box_size); ++i) {
        for (int j = std::max(1, binyMax-box_size); j <= std::min(nbinsY, binyMax+box_size); ++j) {
            double w = h2->GetBinContent(i,j);
            if (w <= 0) continue;
            double x = h2->GetXaxis()->GetBinCenter(i);
            double y = h2->GetYaxis()->GetBinCenter(j);
            sum_w += w;
            sum_x += w*x;
            sum_y += w*y;
            sum_x2 += w*x*x;
            sum_y2 += w*y*y;
        }
    }

    if (sum_w > 0) {
        xCenter = sum_x/sum_w;
        yCenter = sum_y/sum_w;
        xRMS = std::sqrt(sum_x2/sum_w - xCenter*xCenter);
        yRMS = std::sqrt(sum_y2/sum_w - yCenter*yCenter);
    } else {
        // fallback to global mean and RMS
        xCenter = h2->GetMean(1);
        yCenter = h2->GetMean(2);
        xRMS = h2->GetRMS(1);
        yRMS = h2->GetRMS(2);
    }
}

// ---------------------
// Main program
// ---------------------
int main() {

    const char* filename = "../output/histogram_data.root";
    const char* outdat   = "../output/fit_results_dblgauss_positive.dat";

    TFile *f = TFile::Open(filename, "READ");
    if (!f || f->IsZombie()) {
        std::cerr << "Error: cannot open " << filename << std::endl;
        return 1;
    }

    std::ofstream fout(outdat);
    fout << "#HistName  "
         << "Amp1 dAmp1  Amp2 dAmp2  X0 dX0  Y0 dY0  SigmaX1 dSigmaX1  SigmaY1 dSigmaY1  "
         << "SigmaX2 dSigmaX2  SigmaY2 dSigmaY2  Theta dTheta  Const dConst  "
         << "Ax dAx  Ay dAy  Cxy dCxy  Chi2 NDF\n";

    TIter nextkey(f->GetListOfKeys());
    TKey *key;
    while ((key = (TKey*)nextkey())) {
        TObject *obj = key->ReadObj();
        if (!obj->InheritsFrom("TH2")) continue;

        TH2 *h2 = dynamic_cast<TH2*>(obj);
        if (!h2) continue;

        std::cout << "Fitting " << h2->GetName() << std::endl;

        // --- Smooth histogram mildly ---
        TH2 *h2sm = (TH2*)h2->Clone("h2sm");
        h2sm->Smooth(1);

        // --- Find max bin ---
        int binxMax, binyMax, binzMax;
        h2sm->GetMaximumBin(binxMax, binyMax, binzMax);

        double xCenter, yCenter, xRMS, yRMS;
        WeightedCentroid(h2sm, binxMax, binyMax, 10, xCenter, yCenter, xRMS, yRMS);

        delete h2sm;

        // --- Fit region: ±3×RMS around centroid ---
        double xmin = xCenter - 3*xRMS;
        double xmax = xCenter + 3*xRMS;
        double ymin = yCenter - 3*yRMS;
        double ymax = yCenter + 3*yRMS;

        TF2 *f2 = new TF2("f2", TiltedDoubleGauss2D, xmin, xmax, ymin, ymax, 13);
        f2->SetNpx(100); f2->SetNpy(100);

        // --- Initial guesses ---
        Double_t initParams[13] = {
            h2->GetMaximum(), 0.3*h2->GetMaximum(), // Amp1, Amp2
            xCenter, yCenter,                       // X0, Y0
            xRMS, yRMS,                             // SigmaX1, SigmaY1
            2.0*xRMS, 2.0*yRMS,                     // SigmaX2, SigmaY2
            TMath::Pi()/4,                           // Theta inside π/6–π/3
            0.0, 0.0, 0.0, 0.0                      // Const, Ax, Ay, Cxy
        };
        f2->SetParameters(initParams);

        // --- Set parameter limits ---
        f2->SetParLimits(0, 0.0, 10*h2->GetMaximum());  // Amp1
        f2->SetParLimits(1, 0.0, 10*h2->GetMaximum());  // Amp2
        f2->SetParLimits(2, xCenter - 1.5*xRMS, xCenter + 1.5*xRMS); // X0
        f2->SetParLimits(3, yCenter - 1.5*yRMS, yCenter + 1.5*yRMS); // Y0
        f2->SetParLimits(4, 0.01, 2.0*xRMS); // SigmaX1
        f2->SetParLimits(5, 0.01, 2.0*yRMS); // SigmaY1
        f2->SetParLimits(6, 0.01, 3.0*xRMS); // SigmaX2
        f2->SetParLimits(7, 0.01, 3.0*yRMS); // SigmaY2
        f2->SetParLimits(8, TMath::Pi()/6, TMath::Pi()/3); // Theta

        // --- Draw histogram + fit box ---
        TCanvas *c = new TCanvas("c", h2->GetName(), 800, 600);
        h2->Draw("COLZ");

        TBox *box = new TBox(xmin, ymin, xmax, ymax);
        box->SetLineColor(kRed);
        box->SetLineWidth(2);
        box->SetFillStyle(0);
        box->Draw();

        // --- Fit ---
        h2->Fit(f2, "R"); // Fit in range only

        f2->SetLineColor(kBlue);
        f2->Draw("SAME");
        c->Update();

        // --- Save results ---
        fout << h2->GetName();
        for (int i=0; i<13; i++) {
            fout << "  " << f2->GetParameter(i) << "  " << f2->GetParError(i);
        }
        fout << "  " << f2->GetChisquare() << "  " << f2->GetNDF() << "\n";

        // --- Save canvas ---
        TString safeName = h2->GetName();
        safeName.ReplaceAll("/", "_");
        TString outpng = TString("../output/") + safeName + "_fit.png";
        c->SaveAs(outpng);

        delete f2;
        delete c;
    }

    fout.close();
    f->Close();
    std::cout <<"Results written to " << outdat << std::endl;

    return 0;
}
