#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"
#include "TTree.h"
#include "TLegend.h"
#include "TPaveText.h"
#include "TMultiGraph.h"
#include "TStyle.h"
#include "TMath.h"
#include "TString.h"
#include "TSystem.h"
#include "TLatex.h"
#include "TPad.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

// Compton formula functions
Double_t ComptonEnergy(Double_t k0, Double_t theta_deg) {
    Double_t theta = theta_deg * TMath::Pi() / 180.0;
    return k0 / (1.0 + (k0/511.0) * (1.0 - TMath::Cos(theta)));
}

Double_t dE_dtheta(Double_t k0, Double_t theta_deg) {
    Double_t theta = theta_deg * TMath::Pi() / 180.0;
    Double_t denominator = 1.0 + (k0/511.0) * (1.0 - TMath::Cos(theta));
    return (k0*k0 / 511.0) * TMath::Sin(theta) / (denominator * denominator) * TMath::Pi()/180.0;
}

// Theoretical Compton function for fitting
Double_t ComptonTheory(Double_t *x, Double_t *par) {
    // par[0] = k0 (initial photon energy)
    return ComptonEnergy(par[0], x[0]);
}

void ComptonAnalysis() {
    
    // Set ROOT style
    gStyle->SetOptStat(0);
    gStyle->SetPadGridX(kTRUE);
    gStyle->SetPadGridY(kTRUE);
    gStyle->SetGridStyle(3);
    gStyle->SetGridWidth(1);
    gStyle->SetGridColor(kGray);
    
    // Configuration
    TString output_dir = "output_cb2d_scipyfit_overlay";
    TString fit_file = output_dir + "/fit_parameters.dat";
    Double_t angle_error = 5.0; // degrees
    Double_t k0_theory = 511.0; // keV
    
    // Vectors to store data
    std::vector<Double_t> angles, E1, sigma_E1, E2, sigma_E2, integrated_rates;
    std::vector<Double_t> angle_errors, sigma_E1_total, sigma_E2_total, sigma_sum;
    
    // Read data from file
    std::ifstream file(fit_file.Data());
    if (!file.is_open()) {
        std::cout << "Error: Cannot open file " << fit_file << std::endl;
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line[0] == '#') continue; // Skip comments
        
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        if (tokens.size() < 15) continue; // Ensure we have all columns
        
        Double_t angle = std::stod(tokens[0]);
        Double_t mu_x = std::stod(tokens[2]);
        Double_t sig_x = std::stod(tokens[3]);
        Double_t mu_y = std::stod(tokens[6]);
        Double_t sig_y = std::stod(tokens[7]);
        Double_t integrated = (tokens.size() > 15) ? std::stod(tokens[15]) : 0.0;
        
        angles.push_back(angle);
        E1.push_back(mu_x);
        sigma_E1.push_back(sig_x);
        E2.push_back(mu_y);
        sigma_E2.push_back(sig_y);
        integrated_rates.push_back(integrated);
        angle_errors.push_back(angle_error);
        
        // Calculate angle uncertainty contributions
        Double_t sigma_angle_E1 = dE_dtheta(k0_theory, angle) * angle_error;
        Double_t sigma_angle_E2 = dE_dtheta(k0_theory, angle) * angle_error;
        
        // Total uncertainties
        Double_t sig_E1_tot = TMath::Sqrt(sig_x*sig_x + sigma_angle_E1*sigma_angle_E1);
        Double_t sig_E2_tot = TMath::Sqrt(sig_y*sig_y + sigma_angle_E2*sigma_angle_E2);
        Double_t sig_sum_tot = TMath::Sqrt(sig_E1_tot*sig_E1_tot + sig_E2_tot*sig_E2_tot);
        
        sigma_E1_total.push_back(sig_E1_tot);
        sigma_E2_total.push_back(sig_E2_tot);
        sigma_sum.push_back(sig_sum_tot);
    }
    file.close();
    
    Int_t npoints = angles.size();
    if (npoints == 0) {
        std::cout << "Error: No data points loaded!" << std::endl;
        return;
    }
    
    std::cout << "Loaded " << npoints << " data points" << std::endl;
    
    // Calculate energy sums and deviations
    std::vector<Double_t> Esum, deviation;
    Double_t sum_dev = 0.0, sum_dev2 = 0.0;
    
    for (Int_t i = 0; i < npoints; i++) {
        Double_t esum = E1[i] + E2[i];
        Double_t dev = k0_theory - esum;
        Esum.push_back(esum);
        deviation.push_back(dev);
        sum_dev += dev;
        sum_dev2 += dev * dev;
    }
    
    Double_t avg_deviation = sum_dev / npoints;
    Double_t rms_deviation = TMath::Sqrt(sum_dev2 / npoints);
    Double_t std_deviation = TMath::Sqrt((sum_dev2 - sum_dev*sum_dev/npoints) / (npoints-1));
    
    std::cout << "Average deviation: " << avg_deviation << " ± " << std_deviation << " keV" << std::endl;
    std::cout << "RMS deviation: " << rms_deviation << " keV" << std::endl;
    
    // Create TGraphErrors objects
    TGraphErrors *gr_E1 = new TGraphErrors(npoints, &angles[0], &E1[0], &angle_errors[0], &sigma_E1_total[0]);
    TGraphErrors *gr_E2 = new TGraphErrors(npoints, &E2[0], &E2[0], &angle_errors[0], &sigma_E2_total[0]);
    TGraphErrors *gr_Esum = new TGraphErrors(npoints, &angles[0], &Esum[0], &angle_errors[0], &sigma_sum[0]);
    TGraphErrors *gr_deviation = new TGraphErrors(npoints, &angles[0], &deviation[0], &angle_errors[0], &sigma_sum[0]);
    
    // Set graph properties
    gr_E1->SetMarkerStyle(20);
    gr_E1->SetMarkerColor(kRed);
    gr_E1->SetLineColor(kRed);
    gr_E1->SetMarkerSize(0.8);
    gr_E1->SetName("gr_E1");
    gr_E1->SetTitle("E_{1} measured");
    
    gr_E2->SetMarkerStyle(21);
    gr_E2->SetMarkerColor(kBlue);
    gr_E2->SetLineColor(kBlue);
    gr_E2->SetMarkerSize(0.8);
    gr_E2->SetName("gr_E2");
    gr_E2->SetTitle("E_{2} measured");
    
    gr_Esum->SetMarkerStyle(22);
    gr_Esum->SetMarkerColor(kGreen+2);
    gr_Esum->SetLineColor(kGreen+2);
    gr_Esum->SetMarkerSize(0.8);
    
    gr_deviation->SetMarkerStyle(20);
    gr_deviation->SetMarkerColor(kRed);
    gr_deviation->SetLineColor(kRed);
    gr_deviation->SetMarkerSize(0.8);
    
    // Create theoretical curves
    TF1 *f_E1_theory = new TF1("f_E1_theory", ComptonTheory, 0, 180, 1);
    f_E1_theory->SetParameter(0, k0_theory);
    f_E1_theory->SetLineColor(kRed);
    f_E1_theory->SetLineStyle(2);
    f_E1_theory->SetLineWidth(2);
    
    TF1 *f_E2_theory = new TF1("f_E2_theory", "[0] - ComptonEnergy([0], x)", 0, 180);
    f_E2_theory->SetParameter(0, k0_theory);
    f_E2_theory->SetLineColor(kBlue);
    f_E2_theory->SetLineStyle(2);
    f_E2_theory->SetLineWidth(2);
    
    // Alternative way to create E2 theory curve
    TGraph *gr_E2_theory = new TGraph();
    for (Int_t i = 0; i <= 180; i++) {
        Double_t E1_th = ComptonEnergy(k0_theory, i);
        Double_t E2_th = k0_theory - E1_th;
        gr_E2_theory->SetPoint(i, i, E2_th);
    }
    gr_E2_theory->SetLineColor(kBlue);
    gr_E2_theory->SetLineStyle(2);
    gr_E2_theory->SetLineWidth(2);
    
    // Create comprehensive canvas
    TCanvas *c1 = new TCanvas("c1", "Compton Scattering Analysis", 1200, 900);
    c1->Divide(2, 2);
    
    // Panel 1: Main E1 and E2 plot
    c1->cd(1);
    gPad->SetGrid();
    
    TMultiGraph *mg = new TMultiGraph();
    mg->Add(gr_E1, "P");
    mg->Add(gr_E2, "P");
    mg->Draw("A");
    mg->SetTitle("Compton Scattering: Energy vs Angle;Scattering angle #theta (deg);Energy (keV)");
    mg->GetXaxis()->SetRangeUser(0, 180);
    mg->GetYaxis()->SetRangeUser(0, 550);
    
    f_E1_theory->Draw("same");
    gr_E2_theory->Draw("L same");
    
    TLegend *leg1 = new TLegend(0.6, 0.6, 0.89, 0.89);
    leg1->AddEntry(gr_E1, "E_{1} measured", "p");
    leg1->AddEntry(gr_E2, "E_{2} measured", "p");
    leg1->AddEntry(f_E1_theory, "E_{1} theory", "l");
    leg1->AddEntry(gr_E2_theory, "E_{2} theory", "l");
    leg1->Draw();
    
    // Inset for energy sum
    TPad *inset = new TPad("inset", "inset", 0.15, 0.15, 0.55, 0.45);
    inset->SetFillColor(kWhite);
    inset->SetBorderMode(1);
    inset->Draw();
    inset->cd();
    inset->SetGrid();
    
    gr_Esum->Draw("AP");
    gr_Esum->SetTitle(Form("Avg dev: %.2f #pm %.2f keV", avg_deviation, std_deviation));
    gr_Esum->GetXaxis()->SetTitle("#theta (deg)");
    gr_Esum->GetYaxis()->SetTitle("Energy (keV)");
    gr_Esum->GetXaxis()->SetTitleSize(0.06);
    gr_Esum->GetYaxis()->SetTitleSize(0.06);
    gr_Esum->GetXaxis()->SetLabelSize(0.05);
    gr_Esum->GetYaxis()->SetLabelSize(0.05);
    
    TF1 *f_511 = new TF1("f_511", "511", 0, 180);
    f_511->SetLineColor(kBlack);
    f_511->SetLineStyle(2);
    f_511->Draw("same");
    
    TLegend *leg_inset = new TLegend(0.15, 0.75, 0.8, 0.9);
    leg_inset->AddEntry(gr_Esum, "E_{1}+E_{2}", "p");
    leg_inset->AddEntry(f_511, "511 keV", "l");
    leg_inset->SetTextSize(0.05);
    leg_inset->Draw();
    
    c1->cd();
    
    // Panel 2: Residuals
    c1->cd(2);
    gPad->SetGrid();
    
    // Calculate residuals
    TGraphErrors *gr_res_E1 = new TGraphErrors(npoints);
    TGraphErrors *gr_res_E2 = new TGraphErrors(npoints);
    
    for (Int_t i = 0; i < npoints; i++) {
        Double_t E1_theory_val = ComptonEnergy(k0_theory, angles[i]);
        Double_t E2_theory_val = k0_theory - E1_theory_val;
        
        gr_res_E1->SetPoint(i, angles[i], E1[i] - E1_theory_val);
        gr_res_E1->SetPointError(i, angle_errors[i], sigma_E1_total[i]);
        
        gr_res_E2->SetPoint(i, angles[i], E2[i] - E2_theory_val);
        gr_res_E2->SetPointError(i, angle_errors[i], sigma_E2_total[i]);
    }
    
    gr_res_E1->SetMarkerStyle(20);
    gr_res_E1->SetMarkerColor(kRed);
    gr_res_E1->SetLineColor(kRed);
    gr_res_E2->SetMarkerStyle(21);
    gr_res_E2->SetMarkerColor(kBlue);
    gr_res_E2->SetLineColor(kBlue);
    
    TMultiGraph *mg_res = new TMultiGraph();
    mg_res->Add(gr_res_E1, "P");
    mg_res->Add(gr_res_E2, "P");
    mg_res->Draw("A");
    mg_res->SetTitle("Residuals: Measured - Theory;Scattering angle #theta (deg);Residuals (keV)");
    
    TF1 *f_zero = new TF1("f_zero", "0", 0, 180);
    f_zero->SetLineColor(kBlack);
    f_zero->SetLineStyle(1);
    f_zero->Draw("same");
    
    TLegend *leg2 = new TLegend(0.7, 0.7, 0.89, 0.89);
    leg2->AddEntry(gr_res_E1, "E_{1} residuals", "p");
    leg2->AddEntry(gr_res_E2, "E_{2} residuals", "p");
    leg2->Draw();
    
    // Panel 3: Energy deviation
    c1->cd(3);
    gPad->SetGrid();
    
    gr_deviation->Draw("AP");
    gr_deviation->SetTitle("Energy Conservation: 511 - (E_{1}+E_{2});Scattering angle #theta (deg);Energy deviation (keV)");
    
    f_zero->Draw("same");
    
    // Add uncertainty band
    TGraphErrors *gr_uncertainty_band = new TGraphErrors(npoints);
    for (Int_t i = 0; i < npoints; i++) {
        gr_uncertainty_band->SetPoint(i, angles[i], 0);
        gr_uncertainty_band->SetPointError(i, 0, 2*sigma_sum[i]);
    }
    gr_uncertainty_band->SetFillColor(kGray);
    gr_uncertainty_band->SetFillStyle(3001);
    gr_uncertainty_band->Draw("3 same");
    gr_deviation->Draw("P same");
    
    // Panel 4: Count rates or additional analysis
    c1->cd(4);
    gPad->SetGrid();
    
    Bool_t has_count_rates = kFALSE;
    for (Int_t i = 0; i < npoints; i++) {
        if (integrated_rates[i] > 0) {
            has_count_rates = kTRUE;
            break;
        }
    }
    
    if (has_count_rates) {
        gPad->SetLogy();
        TGraph *gr_rates = new TGraph(npoints, &angles[0], &integrated_rates[0]);
        gr_rates->SetMarkerStyle(20);
        gr_rates->SetMarkerColor(kBlack);
        gr_rates->SetLineColor(kBlack);
        gr_rates->SetMarkerSize(0.8);
        gr_rates->Draw("AP");
        gr_rates->SetTitle("Angular Distribution;Scattering angle #theta (deg);Count rate (counts/s)");
        gr_rates->GetXaxis()->SetRangeUser(0, 180);
    } else {
        // Alternative: Show energy sum vs theory
        TGraph *gr_theory_sum = new TGraph();
        gr_theory_sum->SetPoint(0, 0, 511);
        gr_theory_sum->SetPoint(1, 180, 511);
        gr_theory_sum->SetLineColor(kBlack);
        gr_theory_sum->SetLineStyle(2);
        
        gr_Esum->Draw("AP");
        gr_Esum->SetTitle("Energy Sum vs Theory;Scattering angle #theta (deg);Energy sum (keV)");
        gr_theory_sum->Draw("L same");
    }
    
    // Add text box with statistics
    TPaveText *stats = new TPaveText(0.6, 0.7, 0.89, 0.89, "NDC");
    stats->AddText(Form("Data points: %d", npoints));
    stats->AddText(Form("Avg deviation: %.2f keV", avg_deviation));
    stats->AddText(Form("RMS deviation: %.2f keV", rms_deviation));
    stats->AddText(Form("Std deviation: %.2f keV", std_deviation));
    stats->SetFillColor(kWhite);
    stats->SetBorderSize(1);
    stats->Draw();
    
    c1->Update();
    c1->SaveAs(output_dir + "/compton_analysis_comprehensive.png");
    c1->SaveAs(output_dir + "/compton_analysis_comprehensive.pdf");
    
    // Create simple version (original style)
    TCanvas *c2 = new TCanvas("c2", "Compton Scattering - Simple", 800, 600);
    c2->SetGrid();
    
    TMultiGraph *mg_simple = new TMultiGraph();
    mg_simple->Add(gr_E1, "P");
    mg_simple->Add(gr_E2, "P");
    mg_simple->Draw("A");
    mg_simple->SetTitle("Compton Scattering;Scattering angle #theta (deg);Energy (keV)");
    mg_simple->GetXaxis()->SetRangeUser(0, 180);
    mg_simple->GetYaxis()->SetRangeUser(0, 550);
    
    f_E1_theory->Draw("same");
    gr_E2_theory->Draw("L same");
    
    TLegend *leg_simple = new TLegend(0.6, 0.6, 0.89, 0.89);
    leg_simple->AddEntry(gr_E1, "E_{1} measured", "p");
    leg_simple->AddEntry(gr_E2, "E_{2} measured", "p");
    leg_simple->AddEntry(f_E1_theory, "E_{1} theory", "l");
    leg_simple->AddEntry(gr_E2_theory, "E_{2} theory", "l");
    leg_simple->Draw();
    
    // Simple inset
    TPad *inset2 = new TPad("inset2", "inset2", 0.55, 0.15, 0.89, 0.4);
    inset2->SetFillColor(kWhite);
    inset2->SetBorderMode(1);
    inset2->Draw();
    inset2->cd();
    inset2->SetGrid();
    
    gr_Esum->Draw("AP");
    gr_Esum->SetTitle(Form("Avg dev: %.2f keV", avg_deviation));
    gr_Esum->GetXaxis()->SetTitle("#theta (deg)");
    gr_Esum->GetYaxis()->SetTitle("Energy (keV)");
    gr_Esum->GetXaxis()->SetTitleSize(0.08);
    gr_Esum->GetYaxis()->SetTitleSize(0.08);
    gr_Esum->GetXaxis()->SetLabelSize(0.07);
    gr_Esum->GetYaxis()->SetLabelSize(0.07);
    
    f_511->Draw("same");
    
    TLegend *leg2_simple = new TLegend(0.15, 0.75, 0.8, 0.9);
    leg2_simple->AddEntry(gr_Esum, "E_{1}+E_{2}", "p");
    leg2_simple->AddEntry(f_511, "511 keV", "l");
    leg2_simple->SetTextSize(0.07);
    leg2_simple->Draw();
    
    c2->cd();
    c2->Update();
    c2->SaveAs(output_dir + "/E_vs_theta_with_inset_legend.png");
    c2->SaveAs(output_dir + "/E_vs_theta_with_inset_legend.pdf");
    
    // Save ROOT file with all objects
    TFile *outfile = new TFile(output_dir + "/compton_analysis.root", "RECREATE");
    gr_E1->Write();
    gr_E2->Write();
    gr_Esum->Write();
    gr_deviation->Write();
    gr_res_E1->Write("gr_residuals_E1");
    gr_res_E2->Write("gr_residuals_E2");
    f_E1_theory->Write();
    gr_E2_theory->Write();
    c1->Write();
    c2->Write();
    outfile->Close();
    
    std::cout << "Analysis complete! Files saved to " << output_dir << "/" << std::endl;
    std::cout << "- compton_analysis_comprehensive.png/pdf (detailed analysis)" << std::endl;
    std::cout << "- E_vs_theta_with_inset_legend.png/pdf (simple version)" << std::endl;
    std::cout << "- compton_analysis.root (ROOT file with all objects)" << std::endl;
}
