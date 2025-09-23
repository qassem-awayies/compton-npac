/*
 *  "faster_file_is_sorted.c"
 *
 *  Tests whether data in the file are ordered with respect to time.
 *
 */

#include <stdio.h>
#include <string.h>

#include "fasterac/fasterac.h"                           //  generic data & reader
#include "fasterac/farray.h"                             //  data_array
#include "fasterac/utils.h"                              //  data_display


int main (int argc, char** argv) {

  char*              data_space;                         //  file in memory
  size_t             space_size;
  int                error;
  farray             *far;                               //  faster data array
  unsigned long long c1     = 0;                         //  clock
  unsigned long long c2     = 0;                         //  clock to compare
  int                sorted = 1;                         //  the file is sorted
  int                i;                                  //

  if (argc < 2) {                                        //  command args & usage
    printf ("\nusage : \n");
    printf ("        %s  filename.fast\n", argv[0]);
    printf ("\n");
    return EXIT_SUCCESS;
  }

  error = farray_data_file_to_memory (argv [1], &data_space, &space_size);
  if (error) {                                           // file to memory
    printf ("error opening %s\n", argv [1]);
    return 1;
  }
  far = farray_new (data_space, space_size);             //  create far with data space

  c1 = far->first_ns;                                    //  get first clock
  for (i=1; i < far->nb_data; i++) {                     //  loop on data
    c2 = faster_data_clock_ns (far->data_array [i]);     //  get current clock
    if (c2 < c1) {                                       //  compare current and last clocks
      sorted = 0;                                        //  display swapped data
      printf (" Data swap  %.0Ldns\n", (long long)c1-c2);//
      data_display (far->data_array [i-1], i-1, 0, 0);   //  'data_display' info  => /usr/include/fasterac/utils.h
      data_display (far->data_array [i]  , i  , 0, 0);   //  data fields handling => /usr/include/fasterac/*.h
      printf ("\n");                                     //                       => '*_attributes_str (...)' func.
    }                                                    //
    c1 = c2;                                             //  set last clock
  }

  farray_free (far);                                     //  free allocated memory
  free (data_space);
  if (sorted) printf ("Data file OK\n");
  return EXIT_SUCCESS;

}


