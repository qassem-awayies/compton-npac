#include "Riostream.h"
#include "TTree.h"
#include "TFile.h"

//#include "Oscillo.h"


TTree* load_oscillo_tree (TString osc_tree) {

  //
  //  Requirements:
  //
  //    gROOT->ProcessLine (".L Oscillo.C+");
  //
  //

  TFile* f = new TFile (osc_tree);
  TTree* t = (TTree*) f->Get ("OscTree");     //  tree format (oscillo_faster2tree.C)

  cout << endl << "LOAD TREE: " << endl;
  t->Print();

  return t;

}
