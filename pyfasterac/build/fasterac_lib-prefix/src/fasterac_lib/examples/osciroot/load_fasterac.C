{

  //  use fasterac
  //
  TString bob = gSystem->GetFromPipe ("pkg-config --cflags libfasterac");
  bob.ReplaceAll ("-I", ".include ");
  gROOT->ProcessLine (bob.Data ());
  gSystem->Load      ("libfasterac");


}
