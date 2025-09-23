#include "Riostream.h"
#include "TTree.h"

#include "Oscillo.h"
#include "TH_Osc_Explorer.h"

  //
  //  Requirements:
  //
  //    gROOT->ProcessLine (".L Oscillo.C+");
  //    gROOT->ProcessLine (".L TH_Osc_Explorer.C+");
  //
  //

#define ALLOSC -1


void baseline_explorer_get_x (Oscillo* osc, Bool_t* select, Double_t* x) {
  *select = kTRUE;
  *x      = osc->Baseline (-20.0);
}


void baseline_explorer (TTree* osc_tree, int nb_osc = ALLOSC) {
  cout << endl;
  cout << endl;
  cout << "  OSCIROOT baseline_explorer " << endl;
  cout << endl;
  cout << endl;
  Oscillo*         osc   = new Oscillo ();
  TH_Osc_Explorer* blexp = new TH_Osc_Explorer (osc_tree, osc, &baseline_explorer_get_x, "Baseline", nb_osc);
  //TH_Osc_Explorer* blexp = new TH_Osc_Explorer (osc_tree, osc, &baseline_explorer_get_x, "Baseline", nb_osc);
  //TH_Osc_Explorer* blexp = new TH_Osc_Explorer (osc_tree, osc, &baseline_explorer_get_x, "Baseline", nb_osc, 500, -100.0, 150.0);
  //TH_Osc_Explorer* blexp = new TH_Osc_Explorer (osc_tree, osc, &baseline_explorer_get_x, "Baseline", nb_osc, 1000, -100.0, 150.0);
  blexp->SetLogy ();
}

