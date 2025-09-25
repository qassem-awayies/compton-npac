

{

  gStyle->SetPalette          (1);
  gStyle->SetCanvasBorderMode (1);

  TCanvas *TC = new TCanvas ("TC", "Faster to ROOT examples",150,10,990,660);
  TC->Divide (2, 2, 0.01, 0.01, 0);
  TPad *TC_1 = (TPad*) TC->GetListOfPrimitives()->FindObject("TC_1");
  TPad *TC_2 = (TPad*) TC->GetListOfPrimitives()->FindObject("TC_2");
  TPad *TC_3 = (TPad*) TC->GetListOfPrimitives()->FindObject("TC_3");
  TPad *TC_4 = (TPad*) TC->GetListOfPrimitives()->FindObject("TC_4");

  TFile* f1 = new TFile ("group2tree_data.root");
  TTree* t1 = (TTree*) f1->Get ("DataTree");

  TFile* f2 = new TFile ("group2tree_bis_data.root");
  TTree* t2 = (TTree*) f2->Get ("DataTree");

  cout << endl << endl;
  cout << "  group2tree_data.root -> DataTree :" << endl;
  cout << "  ----------------------------------" << endl << endl;
  t1->Print ();
  cout << endl << endl;
  cout << "  group2tree_bis_data.root -> DataTree :" << endl;
  cout << "  --------------------------------------" << endl << endl;
  t2->Print ();

  cout << endl;
  cout << "  GROUP2TREE"   << endl;
  cout << "  ----------" << endl;

  cout << endl;
  cout << "  PAD1 : Bidim Q3=f(Q1)" << endl;
  TC_1->cd ();
  t1->Draw ("Ch3_Q1:Ch1_Q1", "", "colz", 73);

  cout << endl;
  cout << "  GROUP2TREE_BIS"   << endl;
  cout << "  --------------" << endl;

  cout << endl;
  cout << "  PAD2 : Bidim Q3=f(Q1) with cuts" << endl;
  TC_2->cd ();
  t2->Draw ("Ch3_Q1:Ch1_Q1", "(Ch3_Q1 > -1000) && (Ch1_Q1 > -1000)", "colz", 1000);

  cout << endl;
  cout << "  PAD3 : Monodim Q2 with cut" << endl;
  TC_3->cd ();
  t2->Draw ("Ch2_Q1", "Ch2_Q1 > -1000", "", 1000);

  cout << endl;
  cout << "  PAD4 : Monodim Q4 with cut" << endl;
  TC_4->cd ();
  t2->Draw ("Ch4_Q1", "Ch4_Q1 > -1000", "");

  cout << endl;
  cout << endl;
  cout << " .q to quit ... " << endl;
  cout << endl;




}
