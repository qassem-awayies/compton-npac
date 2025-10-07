#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TLegend.h>
#include <TAxis.h>
#include <TStyle.h>
#include <TGraph.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <iostream>

// --- Compton formula ---
double compton_energy(double E, double theta_deg) {
    double theta = theta_deg * M_PI / 180.0;
    double m = 511.0; // electron rest mass (keV)
    return E / (1.0 + (E/m) * (1 - cos(theta)));
}

void plotEvsAng() {
    gStyle->SetOptStat(0);

    std::ifstream infile("./fit_results_1274.dat");
    if (!infile.is_open()) {
        std::cerr << "Error: could not open fit_results.dat\n";
        return;
    }

    std::string line;
    std::getline(infile, line); // skip header

    std::vector<double> angle, mux, mux_err, muy, muy_err, sigma_x, sigma_y;

    while (std::getline(infile, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        double a, mx, mxerr, sx, sxerr, my, myerr, sy, syerr, Esum, Esumerr, dev;
        ss >> a >> mx >> mxerr >> sx >> sxerr >> my >> myerr >> sy >> syerr >> Esum >> Esumerr >> dev;
        angle.push_back(a);
        mux.push_back(mx);
        mux_err.push_back(mxerr);
        muy.push_back(my);
        muy_err.push_back(myerr);
        sigma_x.push_back(sx);
        sigma_y.push_back(sy);
    }

    int n = angle.size();
    TGraphErrors *gX = new TGraphErrors(n, angle.data(), mux.data(), 0, mux_err.data());
    gX->SetMarkerStyle(20);
    gX->SetMarkerColor(kBlue);
    gX->SetLineColor(kBlue);
    gX->SetTitle(";Angle (deg);Energy (keV)");
   // gX->GetYaxis()->SetRangeUser(-10,700);
    TGraphErrors *gY = new TGraphErrors(n, angle.data(), muy.data(), 0, muy_err.data());
    gY->SetMarkerStyle(21);
    gY->SetMarkerColor(kRed);
    gY->SetLineColor(kRed);

    // --- Theoretical curves with resolution-based bands ---
    int ntheta = 181;
    std::vector<double> th_angle(ntheta), th_energy(ntheta), th_energy_err(ntheta);
    std::vector<double> th_loss(ntheta), th_loss_err(ntheta);

    // Take average resolution from your sigmas
    double avg_sigma = 0.0;
    for (size_t i = 0; i < sigma_x.size(); i++) {
        avg_sigma += 0.5 * (sigma_x[i] + sigma_y[i]);
    }
    avg_sigma /= sigma_x.size(); // ~ detector resolution in keV

    for (int i = 0; i < ntheta; i++) {
        double theta = i;
        th_angle[i] = theta;
        th_energy[i] = compton_energy(1274.0, theta);
        th_energy_err[i] = avg_sigma; // band width
        th_loss[i] = 1274.0 - th_energy[i];
        th_loss_err[i] = avg_sigma;
    }

    // Bands
    TGraphErrors *bandCompton = new TGraphErrors(ntheta, th_angle.data(),
                                                 th_energy.data(), 0, th_energy_err.data());
    bandCompton->SetFillColorAlpha(kGreen+1, 0.25);
    bandCompton->SetLineColor(kGreen+3);
    bandCompton->SetLineWidth(2);

    TGraphErrors *bandLoss = new TGraphErrors(ntheta, th_angle.data(),
                                              th_loss.data(), 0, th_loss_err.data());
    bandLoss->SetFillColorAlpha(kMagenta+1, 0.25);
    bandLoss->SetLineColor(kMagenta+3);
    bandLoss->SetLineWidth(2);

    // Central theory lines
    TGraph *lineCompton = new TGraph(ntheta, th_angle.data(), th_energy.data());
    lineCompton->SetLineColor(kGreen+3);
    lineCompton->SetLineWidth(3);

    TGraph *lineLoss = new TGraph(ntheta, th_angle.data(), th_loss.data());
    lineLoss->SetLineColor(kMagenta+3);
    lineLoss->SetLineWidth(3);
    // --- Define fit function for MuY ---
    TF1 *fitLoss = new TF1("fitLoss",
    [](double *x, double *p){
        double theta = x[0] * M_PI/180.0; // convert deg to rad
        double m = 511.0; // keV
        double E = 511.0;
        double arg =  theta;   // shifted/scaled angle
        double val = (E / (1.0 + (E/m)*(1 - cos(arg))))+p[0];
        return 511.0 - val;},
    0, 180, 2);

    // parameters: p0 = offset, p1 = scale
    fitLoss->SetParNames("AngleOffset", "AngleScale");
    fitLoss->SetParameters(0.0, 1.0);  // initial guesses

    // Fit
    //gY->Fit(fitLoss, "R"); // "R" restricts to range
    fitLoss->SetLineColor(kBlack);
    fitLoss->SetLineWidth(2);

    // --- Plot ---
    TCanvas *c1 = new TCanvas("c1", "E_{Meas}, E_{Meas} and Compton vs Angle", 900, 700);

    gX->Draw("AP SAME");        // mux
   // gY->Draw("P SAME");        // muy
    bandCompton->Draw("3 SAME");   // band
   // bandLoss->Draw("3 SAME");  // band
    lineCompton->Draw("L SAME");
    //lineLoss->Draw("L SAME");   // central line

    // --- Legend ---
    TLegend *leg = new TLegend(0.12, 0.7, 0.3, 0.88);
    leg->AddEntry(gX, "E_{#gamma}' (Measured)", "p");
   // leg->AddEntry(gY, "mu_{y}", "p");
    leg->AddEntry(bandCompton, " E_{#gamma}'(Calculated) #pm resolution", "lf");
    //leg->AddEntry(lineCompton, "E_{#gamma}' (calculated)", "l");
    //leg->AddEntry(bandLoss, "511 - E_{#gamma}' ± resolution", "lf");
    //leg->AddEntry(lineLoss, "511 - E_{#gamma}' (theory)", "l");
    leg->Draw();

    c1->SaveAs("E_vs_angle_1274.pdf");
}

