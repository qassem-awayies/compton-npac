/*
 * This program returns the data count rate of the input file.
 * This simple example show how to read every data, retrieving
 * the associated time stamp and counting them.
 *
 * For library specifications, see 'path/to/include/fasterac/fasterac.h'
 *
 */




#include <stdio.h>
#include <stdlib.h>

#include "fasterac/fasterac.h"


int main (int argc, char** argv) {

  char                 info [256];
  faster_file_reader_p reader;
  faster_data_p        data;
  int                  n = 1;
  long double          clock1;
  long double          clock2;


  // Fasterac version
  fasterac_version_number (info);
  printf ("fasterac %s\n\n", info);


  // Input arguments
  if (argc < 2) {
    printf ("\n");
    printf ("usage : \n");
    printf ("        %s  filename.fast\n", argv[0]);
    printf ("\n");
    return EXIT_SUCCESS;
  }


  /* 1. OPEN
   * -------
   * Create and open a data reader for the input file.
   */
  reader = faster_file_reader_open (argv[1]);
  if (reader == NULL) {
    printf ("error opening file %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  /* 2. READ
   * -------
   * Read the first data and get the time stamp of this data.
   * Read the others, count the number of data and get the last time stamp.
   */
  data = faster_file_reader_next (reader);
  clock1 = faster_data_clock_sec (data);
  while ((data = faster_file_reader_next (reader)) != NULL) {
    n = n + 1;
    clock2 = faster_data_clock_sec (data);
  }

  /* 3. CLOSE
   * --------
   * Close and free the reader
   */
  faster_file_reader_close (reader);


  // Display result
  printf ("\n");
  printf ("nb of data = %d\n", n);
  printf ("\n");
  printf ("first data = %.9Lf s\n", clock1);
  printf ("last data  = %.9Lf s\n", clock2);
  printf ("\n");
  printf ("duration   = %.9Lf s\n", clock2 - clock1);
  printf ("\n");
  printf ("count rate = %.1Lf data/s\n", 1.0 * n / (clock2 - clock1));
  printf ("\n");
  return EXIT_SUCCESS;

}
