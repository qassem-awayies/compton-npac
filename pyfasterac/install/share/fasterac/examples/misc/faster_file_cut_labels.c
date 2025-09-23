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
  unsigned short        cur_label;
  unsigned short        *labels;
  int                   n_labels;
  int                   i;
  int                   ok;

  if (argc < 4) {
    printf ("\n");
    printf ("  %s  :  create a new data file by cutting the input file.\n", argv [0]);
    printf ("\n");
    printf ("  usage : \n");
    printf ("          %s  inputfile.fast  outputfile.fast  cut_label1  [cut_label2  [...]]\n", argv[0]);
    printf ("\n");
    return EXIT_SUCCESS;
  }

  strcpy (in_file,  argv [1]);
  strcpy (out_file, argv [2]);
  n_labels = argc - 3;
  labels   = (unsigned short*) malloc (n_labels * sizeof (unsigned short));
  for (i=0; i<n_labels; i++) labels [i] = atoi (argv [3+i]);

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
     ok        = 1;
     for (i=0; i<n_labels; i++) {
        if (cur_label == labels [i]) {
           ok = 0;
           break;
        }
     }
     if (ok) faster_file_writer_next (writer, data);
  }

  free (labels);
  faster_file_reader_close (reader);
  faster_file_writer_close (writer);
  return EXIT_SUCCESS;

}
