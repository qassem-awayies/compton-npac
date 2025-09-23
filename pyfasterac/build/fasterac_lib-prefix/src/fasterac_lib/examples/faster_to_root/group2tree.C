
/*
 *  The file '/Users/qassem.awayies/Projects/compton-npac/pyfasterac/install/share/fasterac/data/smallgrp.fast' is a faster data file
 *  containing four channels of qdc_tdc data. The channels 1 and 3 are systematically
 *  grouped in this file (coinc).
 *  This program reads the data file, put the grouped channels 1 & 3 in a root tree
 *  and saves the given tree in 'group2tree_data.root'.
 *
 *  Rq : channels 2 and 4 are discarded
 *
 */


//  std includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

//  root includes
#include "TFile.h"
#include "TTree.h"

//  fasterac includes
#include "fasterac/fasterac.h"
#include "fasterac/fast_data.h"
#include "fasterac/qdc_caras.h"
#include "fasterac/group.h"

using namespace std;


#define DATAFILENAME "/Users/qassem.awayies/Projects/compton-npac/pyfasterac/install/share/fasterac/data/smallgrp.fast"
#define ROOTFILENAME "group2tree_data.root"
#define LABEL1  1
#define LABEL3  3



int main (int argc, char** argv) {

  /************/
  /*  FASTER  */
  /************/
  //  file reader
  faster_file_reader_p   reader;
  //  data
  faster_data_p          data;
  unsigned char          alias;
  unsigned short         label;
  //  group data
  faster_buffer_reader_p group_reader;
  void*                  group_buffer;
  unsigned short         lsize;
  faster_data_p          group_data;
  //  qdc tdc data  (from faster group)
  qdc_t_x1               qdc1;
  qdc_t_x1               qdc3;
  /**********/
  /*  ROOT  */
  /**********/
  //  root leaves : channels 1 and 3
  Int_t                  leaf_q1;
  Int_t                  leaf_q3;
  //  root tree
  TTree*                 tree;
  char                   tree_title [256];
  //  root file
  TString                fName = ROOTFILENAME;
  TFile*                 root_file;


  //  print infos
  printf ("\n");
  printf ("  group2tree :\n");
  printf ("     - read faster file '%s'\n", DATAFILENAME);
  printf ("     - get grouped data (labels %d and %d)\n", LABEL1, LABEL3);
  printf ("     - output those data to root file '%s'\n", ROOTFILENAME);
  printf ("\n");

  //  open faster file reader
  reader = faster_file_reader_open (DATAFILENAME);
  if (reader == NULL) {
    printf ("error opening file %s\n", DATAFILENAME);
    return EXIT_FAILURE;
  }

  //  output root file
  root_file = new TFile (fName.Data (), "recreate");
  sprintf (tree_title, "faster to tree root test : group2tree.C");
  tree = new TTree ("DataTree", tree_title);
  tree->Branch ("Ch1_Q1", &leaf_q1, "Ch1_Q1/I");
  tree->Branch ("Ch3_Q1", &leaf_q3, "Ch3_Q1/I");

  // main loop
  while ((data = faster_file_reader_next (reader)) != NULL) {                    //  read each data
    alias = faster_data_type_alias (data);
    lsize = faster_data_load_size  (data);
    if (alias == GROUP_TYPE_ALIAS) {
      group_buffer = faster_data_load_p (data);                                  //  get group data
      group_reader = faster_buffer_reader_open (group_buffer, lsize);            //  open group reader
      while ((group_data = faster_buffer_reader_next (group_reader)) != NULL) {  //  read data inside the group
        label = faster_data_label (group_data);
        if (label == LABEL1) {                                                   //  for that file : label1 => qdc_t_x1 ch1
          faster_data_load (group_data, &qdc1);
        } else if (label == LABEL3) {                                            //                  label3 => qdc_t_x1 ch3
          faster_data_load (group_data, &qdc3);
        }
      }
      faster_buffer_reader_close (group_reader);
      leaf_q1 = qdc1.q1;
      leaf_q3 = qdc3.q1;
      tree->Fill ();
    }
  }

  //  close files & quit
  faster_file_reader_close (reader);
  root_file->Write ();
  root_file->Close ();
  return EXIT_SUCCESS;
}


