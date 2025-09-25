/*
 *  Faster data file cutter =>  create a new data file with data from a given file starting at n1 and ending at n2
 *
 */

#include <stdio.h>
#include <limits.h>

#include "fasterac/fasterac.h"
#include "fasterac/utils.h"



int main (int argc, char** argv) {

  faster_file_reader_p  reader;
  faster_file_writer_p  writer;
  faster_data_p         data;
  int                   i;
  int                   n_first;
  int                   n_last;

  if (argc < 4) {
    printf ("\n");
    printf ("  %s  :  create a new data file by cutting the input file.\n", argv [0]);
    printf ("\n");
    printf ("  usage : \n");
    printf ("          %s  inputfile.fast  n_first  n_last   [cut.fast]\n", argv[0]);
    printf ("\n");
    return EXIT_SUCCESS;
  }
  reader = faster_file_reader_open (argv  [1]);
  if (reader == NULL) {
    printf ("error opening file %s\n", argv [1]);
    return EXIT_FAILURE;
  }
  n_first = atoi (argv [2]);
  n_last  = atoi (argv [3]);
  if (argc > 4) {
    writer = faster_file_writer_open (argv [4]);
  } else {
    writer = faster_file_writer_open ("cut.fast");
  }
  i = 1;
  while ((i < n_first) && ((data = faster_file_reader_next (reader)) != NULL)) {
    i++;
  }
  while ((i <= n_last) && ((data = faster_file_reader_next (reader)) != NULL)) {
    //data_display (data, i, 0, 1);
    faster_file_writer_next (writer, data);
    i++;
  }
  faster_file_reader_close (reader);
  faster_file_writer_close (writer);
  return EXIT_SUCCESS;
}
