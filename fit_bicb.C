#include <TFile.h>
#include <TH2.h>
#include <TF2.h>
#include <TKey.h>
#include <TIterator.h>
#include <TROOT.h>
#include <TCanvas.h>
#include <TBox.h>
#include <TString.h>

#include <fstream>
#include <iostream>
#include <cmath>

// ---------------------
// 1D Crystal Ball
// ---------------------
double crystalBall(double x, double mu, double sigma, double alpha, double n) {
    double t = (x - mu)/sigma;
    if (t > -alpha) return exp(-0.5*t*t);
    else {
        double A = pow(n/alpha, n) * exp(-0.5*alpha*alpha);
        double B = n/alpha - alpha;
        return A * pow(B - t, -n);
    }
}

// ---------------------
// Tilted 2D Crystal Ball + background
// ---------------------
Double_t TiltedCrystalBall2D(Double_t *x, Double_t *par) {
    // par[0]=Amplitude, par[1]=X0, par[2]=Y0,
    // par[3]=sigmaX, par[4]=sigmaY, par[5]=theta,
    // par[6]=Const, par[7]=Ax, par[8]=By, par[9]=Cxy,
    // par[10]=alphaX, par[11]=nX, par[12]=alphaY, par[13]=nY

    double X = x[0];
    double Y = x[1];

    // Rotate coordinates
    double x0 = par[1], y0 = par[2];
    double theta = par[5];
    double xrot =  (X - x0)*cos(theta) + (Y - y0)*sin(theta);
    double yrot = -(X - x0)*sin(theta) + (Y - y0)*cos(theta);

    double gausX = crystalBall(xrot, 0, par[3], par[10], par[11]);
    double gausY = crystalBall(yrot, 0, par[4], par[12], par[13]);
    double gaus = par[0] * gausX * gausY;

    double bg = par[6] + par[7]*X + par[8]*Y + par[9]*X*Y;

    return gaus + bg;
}

// ---------------------
// Main program
// ---------------------
int main() {

    const char* filename = "./output/histogram_data.root";
    const char* outdat   = "./output/fit_results_cb.dat";

    // Open ROOT file
    TFile *f = TFile::Open(filename, "READ");
    if (!f || f->IsZombie()) {
        std::cerr << "Error: cannot open " << filename << std::endl;
        return 1;
    }

    std::ofstream fout(outdat);
    fout << "#HistName  "
         << "Amp dAmp  X0 dX0  Y0 dY0  SigmaX dSigmaX  SigmaY dSigmaY  Theta dTheta  "
         << "Const dConst  Ax dAx  By dBy  Cxy dCxy  "
         << "AlphaX dAlphaX  nX dnX  AlphaY dAlphaY  nY dnY  "
         << "Chi2 NDF\n";

    TIter nextkey(f->GetListOfKeys());
    TKey *key;
    while ((key = (TKey*)nextkey())) {
        TObject *obj = key->ReadObj();
        if (!obj->InheritsFrom("TH2")) continue;

        TH2 *h2 = dynamic_cast<TH2*>(obj);
        if (!h2) continue;

        std::cout << "Fitting " << h2->GetName() << std::endl;

        // --- Determine maximum bin ---
        int binx, biny, binz;
        h2->GetMaximumBin(binx, biny, binz);
        double xMax = h2->GetXaxis()->GetBinCenter(binx);
        double yMax = h2->GetYaxis()->GetBinCenter(biny);

        // --- Fit region around maximum ---
        double fitRangeX = 300.0; // ± units around maximum (adjust as needed)
        double fitRangeY = 300.0;
        double xmin = xMax - fitRangeX;
        double xmax = xMax + fitRangeX;
        double ymin = yMax - fitRangeY;
        double ymax = yMax + fitRangeY;

        TF2 *f2 = new TF2("f2", TiltedCrystalBall2D, xmin, xmax, ymin, ymax, 14);

        // Initial guesses (array for >11 parameters)
        Double_t initParams[14] = {
            h2->GetMaximum(), xMax, yMax,    // Amp, X0, Y0
            h2->GetRMS(1), h2->GetRMS(2), 0.0, // sigmaX, sigmaY, theta
            0.0, 0.0, 0.0, 0.0,             // Const, Ax, By, Cxy
            1.5, 2.0, 1.5, 2.0              // alphaX, nX, alphaY, nY
        };
        f2->SetParameters(initParams);

        // --- Draw histogram + fit region box ---
        TCanvas *c = new TCanvas("c", h2->GetName(), 800, 600);
        h2->Draw("COLZ");

        TBox *box = new TBox(xmin, ymin, xmax, ymax);
        box->SetLineColor(kRed);
        box->SetLineWidth(2);
        box->SetFillStyle(0);
        box->Draw();

        // Fit
        h2->Fit(f2, "R"); // R=use range
        f2->SetLineColor(kBlue);
        //f2->Draw("SAME");
        c->Update();

        // Save canvas
        TString safeName = h2->GetName();
        safeName.ReplaceAll("/", "_");
        TString outpng = TString("./output/") + safeName + "_fit.png";
        c->SaveAs(outpng);

        // Write results
        fout << h2->GetName();
        for (int i=0; i<14; i++) {
            fout << "  " << f2->GetParameter(i) << "  " << f2->GetParError(i);
        }
        fout << "  " << f2->GetChisquare() << "  " << f2->GetNDF() << "\n";

        delete f2;
        delete c;
    }

    fout.close();
    f->Close();
    std::cout << "Results written to " << outdat << std::endl;

    return 0;
}


