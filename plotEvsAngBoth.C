#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TLegend.h>
#include <TAxis.h>
#include <TStyle.h>
#include <TGraph.h>
#include <TF1.h>
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

// --- Helper to read fit results ---
void readFitResults(const std::string& filename,
                    std::vector<double>& angle,
                    std::vector<double>& mux,
                    std::vector<double>& mux_err,
                    std::vector<double>& sigma_x,
                    std::vector<double>& sigma_y) {

    std::ifstream infile(filename);
    if(!infile.is_open()) {
        std::cerr << "Error: could not open " << filename << "\n";
        return;
    }

    std::string line;
    std::getline(infile, line); // skip header

    while(std::getline(infile, line)) {
        if(line.empty()) continue;
        std::stringstream ss(line);
        double a, mx, mxerr, sx, sxerr, my, myerr, sy, syerr, amp, amperr;
        ss >> a >> mx >> mxerr >> sx >> sxerr >> my >> myerr >> sy >> syerr >> amp >> amperr;
        angle.push_back(a);
        mux.push_back(mx);
        mux_err.push_back(mxerr);
        sigma_x.push_back(sx);
        sigma_y.push_back(sy);
    }
}

void plotEvsAngBoth() {
    gStyle->SetOptStat(0);

    // --- Read both datasets ---
    std::vector<double> angle1, mux1, mux_err1, sigma_x1, sigma_y1;
    std::vector<double> angle2, mux2, mux_err2, sigma_x2, sigma_y2;

    readFitResults("fit_results.dat", angle1, mux1, mux_err1, sigma_x1, sigma_y1);
    readFitResults("fit_results_1274.dat", angle2, mux2, mux_err2, sigma_x2, sigma_y2);

    int n1 = angle1.size();
    int n2 = angle2.size();

    // --- Graphs for 511 keV ---
    TGraphErrors *gX_511 = new TGraphErrors(n1, angle1.data(), mux1.data(), 0, mux_err1.data());
    gX_511->SetMarkerStyle(20); // solid circle
    gX_511->SetMarkerColor(kBlue); gX_511->SetLineColor(kBlue);

    // --- Graphs for 1274 keV ---
    TGraphErrors *gX_1274 = new TGraphErrors(n2, angle2.data(), mux2.data(), 0, mux_err2.data());
    gX_1274->SetMarkerStyle(21); // solid square
    gX_1274->SetMarkerColor(kGreen+2); gX_1274->SetLineColor(kGreen+2);

    // --- Theoretical Compton curves ---
    int ntheta = 181;
    std::vector<double> th_angle(ntheta), th_energy_511(ntheta), th_energy_1274(ntheta);
    for(int i=0;i<ntheta;i++){
        th_angle[i] = i;
        th_energy_511[i] = compton_energy(511.0, i);
        th_energy_1274[i] = compton_energy(1274.0, i);
    }

    TGraph *line511 = new TGraph(ntheta, th_angle.data(), th_energy_511.data());
    line511->SetLineColor(kBlue); line511->SetLineWidth(2);

    TGraph *line1274 = new TGraph(ntheta, th_angle.data(), th_energy_1274.data());
    line1274->SetLineColor(kGreen+2); line1274->SetLineWidth(2);

    // --- Resolution bands ---
    auto computeAvgSigma = [](const std::vector<double>& sx, const std::vector<double>& sy){
        double avg = 0.0;
        for(size_t i=0;i<sx.size();i++) avg += 0.5*(sx[i]+sy[i]);
        return avg / sx.size();
    };

    double avg_sigma_511 = computeAvgSigma(sigma_x1,sigma_y1);
    double avg_sigma_1274 = computeAvgSigma(sigma_x2,sigma_y2);

    std::vector<double> band_err_511(ntheta, avg_sigma_511);
    std::vector<double> band_err_1274(ntheta, avg_sigma_1274);

    TGraphErrors *band511 = new TGraphErrors(ntheta, th_angle.data(), th_energy_511.data(), 0, band_err_511.data());
    band511->SetFillColorAlpha(kBlue, 0.25);
    band511->SetLineColor(kBlue); band511->SetLineWidth(1);

    TGraphErrors *band1274 = new TGraphErrors(ntheta, th_angle.data(), th_energy_1274.data(), 0, band_err_1274.data());
    band1274->SetFillColorAlpha(kGreen+2, 0.25);
    band1274->SetLineColor(kGreen+2); band1274->SetLineWidth(1);

    // --- Canvas ---
    TCanvas *c1 = new TCanvas("c1","Compton E vs Angle",900,700);

    // Compute global y-axis range
    double y_min = 1e6, y_max = -1e6;
    auto updateRange = [&](const std::vector<double>& y, const std::vector<double>& yerr){
        for(size_t i=0;i<y.size();i++){
            if(y[i]-yerr[i] < y_min) y_min = y[i]-yerr[i];
            if(y[i]+yerr[i] > y_max) y_max = y[i]+yerr[i];
        }
    };
    updateRange(mux1, mux_err1);
    updateRange(mux2, mux_err2);
    for(int i=0;i<ntheta;i++){
        if(th_energy_511[i]-avg_sigma_511<y_min) y_min = th_energy_511[i]-avg_sigma_511;
        if(th_energy_511[i]+avg_sigma_511>y_max) y_max = th_energy_511[i]+avg_sigma_511;
        if(th_energy_1274[i]-avg_sigma_1274<y_min) y_min = th_energy_1274[i]-avg_sigma_1274;
        if(th_energy_1274[i]+avg_sigma_1274>y_max) y_max = th_energy_1274[i]+avg_sigma_1274;
    }

    gX_511->SetTitle(";Angle (deg);Measured Energy (keV)");
    gX_511->GetYaxis()->SetRangeUser(y_min-10, y_max+10); // add small margin
    gX_511->Draw("AP");
    gX_1274->Draw("P SAME");
    band511->Draw("3 SAME");
    band1274->Draw("3 SAME");
    line511->Draw("L SAME");
    line1274->Draw("L SAME");

    // --- Legend ---
    TLegend *leg = new TLegend(0.12,0.7,0.38,0.88);
    leg->AddEntry(gX_511,"E' 511 keV (measured)","p");
    leg->AddEntry(gX_1274,"E' 1274 keV (measured)","p");
    leg->AddEntry(line511,"Compton 511 keV","l");
    leg->AddEntry(line1274,"Compton 1274 keV","l");
    leg->AddEntry(band511,"#pm detector resolution 511 keV","f");
    leg->AddEntry(band1274,"#pm detector resolution 1274 keV","f");
    leg->Draw();

    c1->SaveAs("E_vs_angle_both_bands.pdf");
}
