//
//  Data file reader and data display example
//
//  How to handle specific data (qdc, adc, oscillo, ...)
//      -> see data specifications in "/usr/include/fasterac/*.h"
//      -> howto access data fields with "function *_attributes_str (...)" in each "*.h"
//
//
//




#include <stdio.h>
#include <stdlib.h>

#include "fasterac/fasterac.h"
#include "fasterac/utils.h"


int main (int argc, char** argv) {

  faster_file_reader_p  reader;                                //  data file reader
  faster_data_p         data;                                  //  data pointer
  int                   n    = 0;                              //  data nb
  int                   tab  = 0;                              //  data display nb tabs
  int                   full = 1;                              //  full data display

  if (argc < 2) {                                              //  command args & usage
    printf ("usage : \n");
    printf ("        %s  filename.fast\n", argv[0]);
    printf ("\n");
    return EXIT_SUCCESS;
  }

  reader = faster_file_reader_open (argv[1]);                  //  create a reader
  if (reader == NULL) {
    printf ("error opening file %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  printf ("\n   DATA NUM    TYPE     LABEL    CLOCK\n\n");
  while ((data = faster_file_reader_next (reader)) != NULL) {  //  loop on data  : read & display each
    n = n + 1;                                                 //
    data_display (data, n, tab, full);                         //  display all data fields :
  }                                                            //     specifications in  "/usr/include/fasterac/*.h"
                                                               //     display howto with "static inline void *_attributes_str (...)"
  faster_file_reader_close (reader);                           //  close the reader
  return EXIT_SUCCESS;

}


