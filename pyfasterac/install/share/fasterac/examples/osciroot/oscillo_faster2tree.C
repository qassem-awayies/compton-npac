#include "Riostream.h"
#include "TString.h"
#include "TFile.h"
#include "TTree.h"

#include "limits.h"


#include "fasterac/fasterac.h"
#include "fasterac/fast_data.h"
#include "Oscillo.h"


void oscillo_faster2tree (TString file_in, UShort_t osc_label, TString file_out="osc_tree.root", Int_t nb_max=INT_MAX) {

  //
  //  Arguments :
  //
  //    file_in   : faster input file
  //    osc_label : oscillo label
  //    file_out  : root output file
  //    nb_max    : nb limitation
  //
  //
  //  Requirements:
  //
  //
  //    gROOT->ProcessLine (".L Oscillo.C+");
  //
  //

  //  open a new root tree file and write the tree
  TFile *ftree = new TFile (file_out, "recreate");
  ftree->cd ();

  //  create a new tree with Oscillo as event class
  Oscillo *osc = new Oscillo ();
  TTree   *mt  = new TTree   ("OscTree", "Oscillo Tree");
  mt->Branch ("Data", "Oscillo", osc, 64000, 0);

  //  open the faster data file
  faster_file_reader_p f_reader;
  faster_data_p        f_data;
  oscillo              o;
  f_reader = faster_file_reader_open (file_in);

  //  read faster file - search oscillo - fill tree
  cout << endl << "CREATE TREE:  please wait ..." << flush;
  Int_t n = 0;
  while ((f_data = faster_file_reader_next (f_reader)) != NULL) {
    if ((faster_data_type_alias (f_data) == SAMPLING_TYPE_ALIAS) && (faster_data_label (f_data) == osc_label)) {
      faster_data_load (f_data, &o);
      if (n==0) {
        short  nb_pts  = faster_data_oscillo_nb_pts (f_data);
        double ns_coef = 0.0;
        char   cap [3] = "--";
        memcpy (cap, o.xcap, 2);
        if      (strcmp (cap, "ns") == 0)
          ns_coef = 1.0;
        else if (strcmp (cap, "us") == 0)
          ns_coef = 1000.0;
        else if (strcmp (cap, "ms") == 0)
          ns_coef = 1000000.0;
        else {
          cout << "error : unknown time unit -" << cap << "-" << endl;
          exit (1);
        }
        memcpy (cap, o.ycap, 2);
        if (strcmp (cap, "mV") != 0) {
          if (strcmp (cap, "AV") != 0) {
            cout << "error : unknown voltage unit -" << cap << "-" << endl;
            exit (1);
          } else {
            memcpy (cap, o.ycap + 4, 2);
            if (strcmp (cap, "mV") != 0) {
              cout << "error : unknown voltage unit -" << cap << "-" << endl;
              exit (1);
            }
          }
        }
        osc->Props_Init (nb_pts, o.x0 * ns_coef, o.xlsb * ns_coef, o.ylsb);
      }
      osc->Points_Init (o.samp);
      mt->Fill();
      n++;
      if (!(n % 50000)) cout << "." << flush;
      if (n >= nb_max) break;
    }
  }
  cout << endl;
  cout << "nb osc = " << n << endl;
  faster_file_reader_close (f_reader);

  //  write tree in file
  ftree->Write();
  ftree->Close();

}

