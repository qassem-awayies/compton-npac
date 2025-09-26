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

    std::ifstream infile("fit_results.dat");
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
    gX->SetTitle("MuX, MuY vs Angle;Angle (deg);Energy (keV)");

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
        th_energy[i] = compton_energy(511.0, theta);
        th_energy_err[i] = avg_sigma; // band width
        th_loss[i] = 511.0 - th_energy[i];
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

    // --- Plot ---
    TCanvas *c1 = new TCanvas("c1", "MuX, MuY and Compton vs Angle", 900, 700);

    bandCompton->Draw("A3");   // band
    bandLoss->Draw("3 SAME");  // band
    lineCompton->Draw("L SAME"); // central line
    lineLoss->Draw("L SAME");   // central line
    gX->Draw("P SAME");        // mux
    gY->Draw("P SAME");        // muy

    // --- Legend ---
    TLegend *leg = new TLegend(0.12, 0.7, 0.48, 0.88);
    leg->AddEntry(gX, "mu_{x}", "p");
    leg->AddEntry(gY, "mu_{y}", "p");
    leg->AddEntry(bandCompton, "Compton E_{#gamma}' ± resolution", "lf");
    leg->AddEntry(lineCompton, "Compton E_{#gamma}' (theory)", "l");
    leg->AddEntry(bandLoss, "511 - E_{#gamma}' ± resolution", "lf");
    leg->AddEntry(lineLoss, "511 - E_{#gamma}' (theory)", "l");
    leg->Draw();

    c1->SaveAs("E_vs_angle.pdf");
}

