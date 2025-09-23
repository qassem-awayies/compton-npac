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

  //
  // specific values fitted to the given example (oscillo14_10k.fast)
  //
#define SAT_14 1180.0   // mV
#define BLT_14  -20.0   // ns
#define BLV_14    0.2   // mV
#define  T1_14  -14.0   // ns
#define  T2_14   14.0   // ns
#define  T3_14  600.0   // ns


void discri_explorer_get_xy (Oscillo* osc, Bool_t* select, Double_t* x, Double_t* y) {
  //
  //  bidim     =>  discrimination [charge ratio : charge tot]
  //  selection =>  saturation test + baseline test + ratio test
  //
  *select = kFALSE;                                      // oscillo rejected
  //
  Bool_t saturated = osc->max_mV > SAT_14;
  if (!saturated) {                                      //  CUT saturated
    Float_t baseline = osc->Baseline (BLT_14);
    if (fabs (baseline) < BLV_14) {                      //  CUT bad baseline
      Float_t fast  = osc->Charge_mVns (T1_14, T2_14);   //  Q fast
      Float_t slow  = osc->Charge_mVns (T2_14, T3_14);   //  Q slow
      Float_t tot   = fast + slow;                       //  Q tot
      Float_t ratio = slow / tot;                        //  Q ratio
      if ((0 < ratio) and (ratio < 1)) {                 //  CUT impossible ratio
        *y      = ratio;
        *x      = tot;
        *select = kTRUE;                                 //  add oscillo to stat
      }
    }
  }
}


void discri_explorer (TTree* osc_tree, int nb_osc = ALLOSC) {
  cout << endl;
  cout << endl;
  cout << "  OSCIROOT discri_explorer " << endl;
  cout << endl;
  cout << endl;
  Oscillo*         osc   = new Oscillo ();
  TH_Osc_Explorer* blexp = new TH_Osc_Explorer (osc_tree, osc, &discri_explorer_get_xy, "Discri n-g", nb_osc);
  //TH_Osc_Explorer* blexp = new TH_Osc_Explorer (osc_tree, osc, &discri_explorer_get_xy, "Discri n-g", nb_osc, 100, 0.0, 3000.0, 100, 0.0, 0.6);
  //TH_Osc_Explorer* blexp = new TH_Osc_Explorer (osc_tree, osc, &discri_explorer_get_xy, "Discri n-g", nb_osc, 250, 0.0, 25000.0, 100, 0.5, 1.0);
  //TH_Osc_Explorer* blexp = new TH_Osc_Explorer (osc_tree, osc, &discri_explorer_get_xy, "Discri n-g", nb_osc, 250, 0.0, 25000.0, 100, 0.3, 1.0);
  blexp->SetLogz ();
  blexp->SetColz ();
}
