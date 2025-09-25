
{

  gStyle->SetPalette          (1);
  gStyle->SetCanvasBorderMode (1);


  //
  //  use fasterac  (lib & inc)
  //
  gROOT->ProcessLine (".x load_fasterac.C");
  cout << "  load fasterac" << endl << endl;

  //
  //  create & load osciroot libs
  //
  cout << endl << "LOAD LIBS:" << endl;

  gROOT->ProcessLine (".L Oscillo.C+");
  cout << "  Oscillo.C" << endl;

  gROOT->ProcessLine (".L oscillo_faster2tree.C+");
  cout << "  oscillo_faster2tree.C" << endl;

  gROOT->ProcessLine (".L load_oscillo_tree.C+");
  cout << "  load_oscillo_tree.C" << endl;

  gROOT->ProcessLine (".L oscillo_test.C+");
  cout << "  oscillo_test.C" << endl;

  gROOT->ProcessLine (".L TH_Osc_Explorer.C+");
  cout << "  TH_Osc_Explorer.C" << endl;

  gROOT->ProcessLine (".L satur_explorer.C+");
  cout << "  satur_explorer.C" << endl;

  gROOT->ProcessLine (".L baseline_explorer.C+");
  cout << "  baseline_explorer.C" << endl;

  gROOT->ProcessLine (".L discri_explorer.C+");
  cout << "  discri_explorer.C" << endl;

  gROOT->ProcessLine (".L Discrillo.C+");
  cout << "  Discrillo.C" << endl;

  gROOT->ProcessLine (".L discri_explorer2.C+");
  cout << "  discri_explorer2.C" << endl;



  //
  //  CREATE A TREE FILE
  //

  oscillo_faster2tree ("/Users/qassem.awayies/Projects/compton-npac/pyfasterac/install/share/fasterac/data/oscillo14_10k.fast", 14, "oscillo14_10k.root");
  //  comment the line once your root file is created



  //
  //  LOAD A TREE
  //

  TTree* t = (TTree*) load_oscillo_tree ("oscillo14_10k.root");



  //
  //  WORK ON IT
  //
  //  comment/uncomment lines to try each & look at the corresponding .C files ...
  //

  Oscillo_Test      (t);       //  from oscillo_test.C
  //satur_explorer    (t);       //  from satur_explorer.C
  //baseline_explorer (t);       //  from baseline_explorer.C
  //discri_explorer   (t);       //  ...
  //discri_explorer2  (t);       //




}
