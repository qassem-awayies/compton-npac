#include "Riostream.h"
#include "TTree.h"

//#include "Oscillo.h"

#define ALLOSC 1000000000

//
//  WARNING
//  constants are specific values fitted to the given example (oscillo14_10k.fast)
//
//  saturation          1180.0 mV
//  baseline end         -20.0 ns
//  baseline limits    +/- 0.2 mV
//  charge window limits -14.0 ns
//                        14.0 ns
//                       600.0 ns
//

void Oscillo_Test (TTree* oscillo_tree, int nb_osc = ALLOSC) {
  //  ---------------------------------------------------------
  //  TTree SetAlias & Draw :
  //    http://root.cern.ch/root/html/TTree.html#TTree:Draw@1
  //    http://root.cern.ch/root/html/TTree.html#TTree:SetAlias
  //  ---------------------------------------------------------

  //
  //  Requirements:
  //
  //    gROOoscillo_tree->ProcessLine (".L Oscillo.C+");
  //
  //

  oscillo_tree->SetAlias ("saturated" ,  "(max_mV > 1180.0)");             //  saturated oscillo

  oscillo_tree->SetAlias ("BL",          "Baseline(-20.0)");
  oscillo_tree->SetAlias ("bad_BL",      "((BL < -0.2) || (0.2 < BL))");   //  bad base line

  oscillo_tree->SetAlias ("fast_Q",      "Charge_mVns(-14.0, 14.0)");
  oscillo_tree->SetAlias ("slow_Q",      "Charge_mVns(14.0, 600.0)");
  oscillo_tree->SetAlias ("tot_Q",       "(fast_Q + slow_Q)");
  oscillo_tree->SetAlias ("ratio_Q",     "(slow_Q / tot_Q)");

  oscillo_tree->SetAlias ("ratio_range", "((0 < ratio_Q) && (ratio_Q < 1))");

  cout << endl;
  cout << endl;
  cout << "  OSCIROOT TTree Alias & Draw tests" << endl;
  cout << endl;
  cout << "  Please Wait ..." << endl;
  cout << endl;
  cout << endl;

  //  -----------------------------------------------------
  //    comment/uncomment the following lines to try each
  //  -----------------------------------------------------

  oscillo_tree->Draw ("max_mV", "", "", nb_osc);
  //oscillo_tree->Draw ("BL",     "", "", nb_osc);
  //oscillo_tree->Draw ("ratio_Q : tot_Q", "ratio_range && !saturated && !bad_BL", "", nb_osc);
  //oscillo_tree->Draw ("ratio_Q : tot_Q", "ratio_range && !saturated && !bad_BL", "colz", nb_osc);

}
