#define TH_Osc_Explorer_cxx

#include "TH_Osc_Explorer.h"

#include <stdlib.h>

#include <Riostream.h>
#include <TStyle.h>
#include <TPaveText.h>
#include <TLegend.h>
#include <TMath.h>


TH_Osc_Explorer::TH_Osc_Explorer (TTree* osc_tree,
                                  Oscillo* osc,
                                  void (*oscillo_get_X) (Oscillo* osc, Bool_t* select, Double_t* x),
                                  const char* title,
                                  Int_t nb_osc_max,
                                  Int_t nbinsx, Double_t xlow, Double_t xup) {
  cout << "Please wait ..." << endl;
  this->Part1_Ctor    (osc_tree, osc, nb_osc_max);
  this->Part2_1D_Ctor (title, nbinsx, xlow, xup, oscillo_get_X);
  this->Part3_Ctor    (osc);
}


TH_Osc_Explorer::TH_Osc_Explorer (TTree* osc_tree,
                                  Oscillo* osc,
                                  void (*oscillo_get_XY) (Oscillo* osc, Bool_t* select, Double_t* x, Double_t* y),
                                  const char* title,
                                  Int_t nb_osc_max,
                                  Int_t nbinsx, Double_t xlow, Double_t xup,
                                  Int_t nbinsy, Double_t ylow, Double_t yup) {
  cout << "Please wait ..." << endl;
  this->Part1_Ctor    (osc_tree, osc, nb_osc_max);
  this->Part2_2D_Ctor (title, nbinsx, xlow, xup, nbinsy, ylow, yup, oscillo_get_XY);
  this->Part3_Ctor    (osc);
}


void TH_Osc_Explorer::Part1_Ctor (TTree* osc_tree, Oscillo* osc, Int_t nb_osc_max) {
  this->tree       = osc_tree;
  this->cur_osc    = osc;
  this->nb_max     = this->tree->GetEntries ();
  if ((nb_osc_max > 0) and (nb_osc_max < this->nb_max)) this->nb_max = nb_osc_max;
  this->nb_entries = 0;
  this->entries    = new TreeEntry [this->nb_max];
  this->first_sel  = -1;
  this->last_sel   = -1;
  this->cur_sel    = -1;
  this->tree->SetBranchAddress("Data", &this->cur_osc);
  TH1::SetDefaultBufferSize (this->nb_max);
}


void TH_Osc_Explorer::Part2_1D_Ctor (const char* title,
                                     Int_t nbinsx, Double_t xlow, Double_t xup,
                                     void (*oscillo_get_X) (Oscillo* osc, Bool_t* select, Double_t* x)) {
  this->bidim      = kFALSE;
  this->osc_get_X  = oscillo_get_X;
  this->osc_get_XY = NULL;
  this->histo      = new TH1F ("histo", title, nbinsx, xlow, xup);
  Double_t xval;
  Bool_t   selected;
  for (int entry=0; entry<this->nb_max; entry++) {
    this->tree->GetEntry (entry);
    this->osc_get_X (this->cur_osc, &selected, &xval);
    if (selected) {
      this->entries [this->nb_entries].entry = entry;
      this->entries [this->nb_entries].x     = xval;
      this->entries [this->nb_entries].y     = 0;
      this->histo->Fill (xval);
      this->nb_entries++;
    }
  }
}


void TH_Osc_Explorer::Part2_2D_Ctor (const char* title,
                                     Int_t nbinsx, Double_t xlow, Double_t xup,
                                     Int_t nbinsy, Double_t ylow, Double_t yup,
                                     void (*oscillo_get_XY) (Oscillo* osc, Bool_t* select, Double_t* x, Double_t* y)) {
  this->bidim      = kTRUE;
  this->osc_get_X  = NULL;
  this->osc_get_XY = oscillo_get_XY;
  this->histo      = new TH2F ("histo", title, nbinsx, xlow, xup, nbinsy, ylow, yup);
  Double_t xval;
  Double_t yval;
  Bool_t   selected;
  for (int entry=0; entry<this->nb_max; entry++) {
    this->tree->GetEntry (entry);
    this->osc_get_XY (this->cur_osc, &selected, &xval, &yval);
    if (selected) {
      this->entries [this->nb_entries].entry = entry;
      this->entries [this->nb_entries].x     = xval;
      this->entries [this->nb_entries].y     = yval;
      this->histo->Fill (xval, yval);
      this->nb_entries++;
    }
  }
}


void TH_Osc_Explorer::Part3_Ctor (Oscillo* osc) {
  //
  qsort (this->entries, this->nb_entries, sizeof (TH_Osc_Explorer::TreeEntry), TH_Osc_Explorer::TreeEntry_X_Compare);
  //
  this->histo_osc  = new TH1F ("H1_osc", "Current Oscillo", osc->nb_pts, osc->x0_ns, osc->x0_ns + osc->nb_pts * osc->n2ns);
  this->histo_osc->GetXaxis ()->SetTitle ("ns");
  this->histo_osc->GetYaxis ()->SetTitle ("mV");
  //
  this->top = (TCanvas*) gROOT->FindObject ("top");
  if (!this->top) this->top = new TCanvas ("top", "TH_Osc_Explorer");
  this->top->Clear  ();
  this->top->Divide (1, 2);
  //
  this->top->cd ();
  this->top->cd (1);
  char click_func [256];
  sprintf (click_func, "((TH_Osc_Explorer*) %lu)->Click_Selection()", (unsigned long int) this);
  gPad->AddExec  ("exe", click_func);
  this->histo->Draw ();
  this->histo_pos1 = new TMarker (0, 0, kOpenCircle);
  this->histo_pos1->SetMarkerColor (1);
  this->histo_pos1->SetMarkerSize  (2.1);
  this->histo_pos1->Draw ();
  this->histo_pos2 = new TMarker (0, 0, kPlus);
  this->histo_pos2->SetMarkerColor (2);
  this->histo_pos2->SetMarkerSize  (1.9);
  this->histo_pos2->Draw ();
  this->info1 = new TPaveText ();
  this->info1->AddText ("Double click on a bin to display corresponding oscillos.");
  this->info1->Draw ();
  //
  this->top->cd ();
  this->top->cd (2);
  sprintf (click_func, "((TH_Osc_Explorer*) %lu)->Click_Next()", (unsigned long int) this);
  gPad->AddExec  ("exe", click_func);
  this->histo_osc->SetStats (0);
  this->histo_osc->Draw ();
  this->info2 = new TPaveText ();
  this->info2->AddText ("Double click to display next oscillo.");
  this->info2->Draw ();
  //
  this->top->Update ();
  //
  Double_t x = this->entries [this->nb_entries - 1].x;
  Double_t y = this->entries [this->nb_entries - 1].y;
  this->Selection (x, y);
}


TH_Osc_Explorer::~TH_Osc_Explorer () {
  delete (this->info1);
  delete (this->info2);
  delete (this->histo_pos1);
  delete (this->histo_pos2);
  delete (this->histo_osc);
  delete (this->histo);
  delete (this->top);
  delete (this->cur_osc);
  delete (this->entries);
}


void TH_Osc_Explorer::SetLogy (Bool_t logy) {
  this->top->cd ();
  this->top->cd (1);
  gPad->SetLogy (logy);
  this->top->Update ();
}


void TH_Osc_Explorer::SetLogz (Bool_t logz) {
  this->top->cd     ();
  this->top->cd     (1);
  gPad->SetLogz     (logz);
  gPad->Modified    ();
  this->top->Update ();
}


void TH_Osc_Explorer::SetColz (Bool_t colz) {
  this->top->cd  ();
  this->top->cd  (1);
  if (colz) {
    this->histo->Draw      ("colz");
    this->histo_pos1->Draw ("colz");
    this->histo_pos2->Draw ("colz");
    this->info1->Draw      ("colz");
  } else {
    this->histo->Draw      ();
    this->histo_pos1->Draw ();
    this->histo_pos2->Draw ();
    this->info1->Draw      ();
  }
  gPad->Modified ();
  this->top->Update ();
}


void TH_Osc_Explorer::Click_Selection () {
  Int_t    n;
  Double_t xx;
  Double_t yy;
  TObject* select;
  Int_t    event = gPad->GetEvent ();
  Int_t    x     = gPad->GetEventX ();
  Int_t    y     = gPad->GetEventY ();
  if (event != 61) return;  // double click
  //
  select = gPad->GetSelected ();
  if (!select) return;
  //
  if (select == this->histo) {
    // bin number from char* TH1F::GetObjectInfo (px, py)    (same as event status bar)
    n   = atoi (strstr (this->histo->GetObjectInfo (x, y), "binc=") + 5);
    if (n==0) return;
    //
    xx  = atof (strstr (this->histo->GetObjectInfo (x, y), "x=") + 2);
    yy  = atof (strstr (this->histo->GetObjectInfo (x, y), "y=") + 2);
    //
    this->Selection (xx, yy);
  }
}


void TH_Osc_Explorer::Selection (Double_t xx, Double_t yy) {
  Int_t    i;
  Int_t    bin;
  Float_t  valx;
  Float_t  valy;
  TAxis*   axis;
  Double_t binLow;
  Double_t binUp;
  //  xbin selection
  axis   = this->histo->GetXaxis ();
  bin    = axis->FindBin         (xx);
  xx     = axis->GetBinCenter    (bin);
  binLow = axis->GetBinLowEdge   (bin);
  binUp  = axis->GetBinUpEdge    (bin);
  this->cur_sel   = -1;
  this->first_sel = -1;
  this->last_sel  = -1;
  for (i=0; i<this->nb_entries; i++) {
    valx = this->entries [i].x;
    if (valx >= binLow) break;
  }
  this->first_sel = i;
  for (i=this->first_sel; i<this->nb_entries; i++) {
    valx = this->entries [i].x;
    if (valx >= binUp) break;
    this->cur_sel = this->last_sel = i;
  }
  //  ybin selection
  if (!this->bidim) {
    yy = this->histo->GetBinContent (bin);
  } else {
    //  sort with y value inside xbin
    qsort (&(this->entries [this->first_sel]),
           this->last_sel - this->first_sel + 1,
           sizeof (TH_Osc_Explorer::TreeEntry),
           TH_Osc_Explorer::TreeEntry_Y_Compare);
    axis   = this->histo->GetYaxis ();
    bin    = axis->FindBin         (yy);
    yy     = axis->GetBinCenter    (bin);
    binLow = axis->GetBinLowEdge   (bin);
    binUp  = axis->GetBinUpEdge    (bin);
    for (i=this->first_sel; i<=this->last_sel; i++) {
      valy = this->entries [i].y;
      if (valy >= binLow) break;
    }
    this->first_sel = i;
    for (i=this->first_sel; i<=this->last_sel; i++) {
      valy = this->entries [i].y;
      if (valy >= binUp) break;
      this->cur_sel = i;
    }
    this->last_sel = this->cur_sel;
  }
  //
  this->top->cd          (1);
  this->histo_pos1->SetX (xx);
  this->histo_pos2->SetX (xx);
  this->histo_pos1->SetY (yy);
  this->histo_pos2->SetY (yy);
  gPad->Modified         ();
  this->top->Update      ();
  this->Display_Next     ();
  //
}


void TH_Osc_Explorer::Click_Next () {
  Int_t    event;
  TObject* select;
  //
  event = gPad->GetEvent ();
  if (event == 61) {  // click
    select = gPad->GetSelected ();
    if (select) this->Display_Next ();
  }
}


void TH_Osc_Explorer::Display_Next () {
    char titl [256];
    //
    if (this->cur_sel < 0) return;
    //
    this->cur_sel++;
    if (this->cur_sel > this->last_sel) this->cur_sel = this->first_sel;
    this->tree->GetEntry (this->entries [this->cur_sel].entry);
    this->cur_osc->Fill  (this->histo_osc);
    sprintf (titl, "Current Oscillo %d/%d", this->cur_sel-this->first_sel+1, this->last_sel-this->first_sel+1);
    this->histo_osc->SetTitle (titl);
    this->top->cd     (2);
    gPad->Modified    ();
    this->top->Update ();
}



int TH_Osc_Explorer::TreeEntry_X_Compare (const void* te1, const void* te2) {
  const Float_t x1 = ((TH_Osc_Explorer::TreeEntry*) te1)->x;
  const Float_t x2 = ((TH_Osc_Explorer::TreeEntry*) te2)->x;
  if (x1 == x2) return 0;
  return (x1 > x2) ? 1 : -1;
}



int TH_Osc_Explorer::TreeEntry_Y_Compare (const void* te1, const void* te2) {
  const Float_t y1 = ((TH_Osc_Explorer::TreeEntry*) te1)->y;
  const Float_t y2 = ((TH_Osc_Explorer::TreeEntry*) te2)->y;
  if (y1 == y2) return 0;
  return (y1 > y2) ? 1 : -1;
}


