#include "Riostream.h"
#include "TTree.h"

#include "Discrillo.h"
#include "TH_Osc_Explorer.h"

  //
  //  Requirements:
  //
  //    gROOT->ProcessLine (".L Oscillo.C+");
  //    gROOT->ProcessLine (".L Discrillo.C+");
  //    gROOT->ProcessLine (".L TH_Osc_Explorer.C+");
  //
  //

#define ALLOSC -1

void discri_explorer2_get_x (Oscillo* osc, Bool_t* select, Double_t* x) {
  //
  Discrillo* osc2 = (Discrillo*) osc;
  //
  *select = kTRUE;
  *x      = osc2->What ();
}


void discri_explorer2 (TTree* osc_tree, int nb_osc = ALLOSC) {
  cout << endl;
  cout << endl;
  cout << "  OSCIROOT discri_explorer2" << endl;
  cout << endl;
  cout << endl;
  Discrillo*       osc2  = new Discrillo ();
  TH_Osc_Explorer* blexp = new TH_Osc_Explorer (osc_tree, osc2, &discri_explorer2_get_x, "Discri : Unknown=0  Gamma=1  Neutron=2", nb_osc);
  blexp->SetLogy (kFALSE);
}
