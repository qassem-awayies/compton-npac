// analyze_results.C - ROOT script to analyze coincidence results
// Run with: root -l analyze_results.C

void analyze_results() {
    // Open the ROOT file
    TFile* file = TFile::Open("Coincidence.root");
    if (!file || file->IsZombie()) {
        cout << "Error: Cannot open Coincidence.root" << endl;
        return;
    }

    // Create a canvas with multiple panels
    TCanvas* c1 = new TCanvas("c1", "Coincidence Analysis", 1200, 800);
    c1->Divide(2, 2);

    // Get histograms
    TH2D* h2_coinc = (TH2D*)file->Get("h2_coinc");
    TH1D* h1_det1 = (TH1D*)file->Get("h1_det1");
    TH1D* h1_det2 = (TH1D*)file->Get("h1_det2");
    TH1D* h1_coinc = (TH1D*)file->Get("h1_coinc");

    // Plot 1: 2D Coincidence map
    c1->cd(1);
    if (h2_coinc) {
        gPad->SetLogz();
        h2_coinc->Draw("COLZ");
        h2_coinc->SetTitle("2D Coincidence Map");
        
        // Draw diagonal line (perfect correlation)
        TLine* line = new TLine(0, 0, 600, 600);
        line->SetLineColor(kRed);
        line->SetLineWidth(2);
        line->Draw("same");
    }

    // Plot 2: Individual detector spectra
    c1->cd(2);
    if (h1_det1 && h1_det2) {
        h1_det1->SetLineColor(kBlue);
        h1_det2->SetLineColor(kRed);
        h1_det1->Draw();
        h1_det2->Draw("same");
        
        TLegend* leg1 = new TLegend(0.7, 0.7, 0.9, 0.9);
        leg1->AddEntry(h1_det1, "Detector 1", "l");
        leg1->AddEntry(h1_det2, "Detector 2", "l");
        leg1->Draw();
        
        gPad->SetLogy();
    }

    // Plot 3: Coincidence sum spectrum
    c1->cd(3);
    if (h1_coinc) {
        h1_coinc->Draw();
        h1_coinc->SetLineColor(kGreen+2);
        h1_coinc->SetTitle("Coincidence Sum Spectrum");
        gPad->SetLogy();
        
        // Add vertical line at 1022 keV (2 x 511 keV)
        TLine* line1022 = new TLine(1022, h1_coinc->GetMinimum(), 
                                   1022, h1_coinc->GetMaximum());
        line1022->SetLineColor(kRed);
        line1022->SetLineStyle(2);
        line1022->Draw("same");
        
        // Add text label
        TText* text = new TText(1050, h1_coinc->GetMaximum()*0.5, "1022 keV");
        text->SetTextColor(kRed);
        text->Draw("same");
    }

    // Plot 4: Statistics and analysis
    c1->cd(4);
    TPaveText* pt = new TPaveText(0.1, 0.1, 0.9, 0.9);
    
    if (h1_det1 && h1_det2 && h1_coinc && h2_coinc) {
        char text[256];
        
        sprintf(text, "Total Events Analyzed:");
        pt->AddText(text);
        
        sprintf(text, "Detector 1 hits: %.0f", h1_det1->GetEntries());
        pt->AddText(text);
        
        sprintf(text, "Detector 2 hits: %.0f", h1_det2->GetEntries());
        pt->AddText(text);
        
        sprintf(text, "Coincidence events: %.0f", h1_coinc->GetEntries());
        pt->AddText(text);
        
        double coinc_efficiency = h1_coinc->GetEntries() / 
                                 std::max(h1_det1->GetEntries(), h1_det2->GetEntries());
        sprintf(text, "Coincidence efficiency: %.3f", coinc_efficiency);
        pt->AddText(text);
        
        // Peak analysis around 511 keV
        int bin511_det1 = h1_det1->FindBin(511);
        int bin511_det2 = h1_det2->FindBin(511);
        sprintf(text, "511 keV peak counts:");
        pt->AddText(text);
        sprintf(text, "Det1: %.0f, Det2: %.0f", 
                h1_det1->GetBinContent(bin511_det1),
                h1_det2->GetBinContent(bin511_det2));
        pt->AddText(text);
        
        // Sum peak analysis
        int bin1022 = h1_coinc->FindBin(1022);
        sprintf(text, "1022 keV sum peak: %.0f", h1_coinc->GetBinContent(bin1022));
        pt->AddText(text);
    }
    
    pt->SetTextAlign(12);
    pt->Draw();

    // Save the canvas
    c1->SaveAs("coincidence_analysis.png");
    c1->SaveAs("coincidence_analysis.pdf");
    
    cout << "\nAnalysis complete! Results saved to:" << endl;
    cout << "- coincidence_analysis.png" << endl;
    cout << "- coincidence_analysis.pdf" << endl;
    
    // Keep ROOT session open
    cout << "\nROOT session remains open for interactive analysis." << endl;
    cout << "Available histograms: h2_coinc, h1_det1, h1_det2, h1_coinc" << endl;
}
