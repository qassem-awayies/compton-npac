/*
 *  Faster data file cutter =>  create a new data file without data selected
 *
 */

#include <stdio.h>
#include <string.h>

#include "fasterac/fasterac.h"
#include "fasterac/utils.h"



int main (int argc, char** argv) {

  char                  in_file  [256];
  char                  out_file [256];
  faster_file_reader_p  reader;
  faster_file_writer_p  writer;
  faster_data_p         data;
  unsigned short        old_label;
  unsigned short        new_label;
  unsigned short        cur_label;

  if (argc != 5) {
    printf ("\n");
    printf ("  %s  :  create a new data file by switching a label.\n", argv [0]);
    printf ("\n");
    printf ("  usage : \n");
    printf ("          %s  inputfile.fast  outputfile.fast  old_label new_label\n", argv[0]);
    printf ("\n");
    return EXIT_SUCCESS;
  }

  strcpy (in_file,  argv [1]);
  strcpy (out_file, argv [2]);
  old_label  = atoi (argv [3]);
  new_label  = atoi (argv [4]);

  reader = faster_file_reader_open (in_file);
  if (reader == NULL) {
    printf ("error opening file %s\n", in_file);
    return EXIT_FAILURE;
  }
  writer = faster_file_writer_open (out_file);
  if (writer == NULL) {
    printf ("error opening file %s\n", out_file);
    return EXIT_FAILURE;
  }

  while ((data = faster_file_reader_next (reader)) != NULL) {
     cur_label = faster_data_label (data);
        if (cur_label == old_label) {
           faster_data_set_label (data, new_label);
        }
     faster_file_writer_next (writer, data);
  }

  faster_file_reader_close (reader);
  faster_file_writer_close (writer);
  return EXIT_SUCCESS;

}
