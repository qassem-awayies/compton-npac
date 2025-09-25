/*
 *  Spectro data in Faster file   =>   Gamma spectrum in ascii file
 *
 *  see also 'spectro_info.c'
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "fasterac/fasterac.h"
#include "fasterac/spectro.h"




int main (int argc, char** argv) {

  faster_file_reader_p  reader;
  faster_data_p         data;
  unsigned char         alias;
  unsigned short        label;
  crrc4_spectro         adc;
  unsigned short        adc_label = 1;
  double                x;
  double                min;
  double                max;
  double                bin_size;
  int                   nb_bins;
  int*                  histo;
  int                   bin_idx;
  FILE*                 specout;
  char                  infilename  [256];
  char                  outfilename [256];

  //  Command line usage
  if (argc < 7) {
    printf ("\n");
    printf ("  %s  :  histogramming CRRC4 Spectro data from Faster file.\n", argv [0]);
    printf ("\n");
    printf ("  usage : \n");
    printf ("          %s  input_file.fast  Spectro_label  min_value  max_value  nb_bins  output_histo.wyw\n", argv[0]);
    printf ("\n");
    printf ("         input_file.fast               : Faster file (containing crrc4_spectro data)\n");
    printf ("         Spectro_label                 : CRRC4_spectro data selection\n");
    printf ("         min_value, max_value, nb_bins : histogram specification\n");
    printf ("         output_histo.wyw              : output two columns ascii file\n");
    printf ("\n");
    printf ("  example : \n");
    printf ("          %s  spectrodata.fast  99  0  40960  4096  ch99_spectrum.txt", argv [0]);
    printf ("\n");
    printf ("\n");
    printf ("  ***  see also 'spectro_info' to extract CRRC4_spectro data infos from a file  *** \n");
    printf ("\n");
    printf ("\n");
    return EXIT_SUCCESS;
  }

  //  Argument parsing
  strcpy (infilename,  argv[1]);
  adc_label = atoi (argv[2]);
  min       = atof (argv[3]);
  max       = atof (argv[4]);
  nb_bins   = atoi (argv[5]);
  strcpy (outfilename, argv[6]);

  //  Histo size, memory allocation and histo reset
  bin_size = (max - min) / nb_bins;
  histo    = (int*) malloc (sizeof (int) * nb_bins);
  for (bin_idx = 0; bin_idx < nb_bins; bin_idx++) histo [bin_idx] = 0;

  //  Input file reader
  reader = faster_file_reader_open (infilename);
  if (reader == NULL) {
    printf ("error opening file %s\n", infilename);
    return EXIT_FAILURE;
  }

  //  Loop on data
  while ((data = faster_file_reader_next (reader)) != NULL) {  //  data reading
    alias = faster_data_type_alias (data);                     //  data type
    label = faster_data_label      (data);                     //  data id
    if (alias==CRRC4_SPECTRO_TYPE_ALIAS && label==adc_label) { //  data selection
      faster_data_load (data, &adc);                           //  get adc specific data part
      if (!adc.saturated && !adc.pileup) {                     //  discard saturated and pile up data
        if (adc.measure>=min && adc.measure<=max) {            //  value selection
          bin_idx = trunc ((adc.measure - min) / bin_size);    //  bin selection
          histo [bin_idx] = histo [bin_idx] + 1;               //  bin incrementation
        }
      }
    }
  }

  //  Reader close
  faster_file_reader_close (reader);

  //  Output file
  specout = fopen (outfilename, "w");
  x = min + bin_size/2;
  for (bin_idx=0; bin_idx<nb_bins; bin_idx++) {
    fprintf (specout, "%f %d\n", x, histo [bin_idx]);
    x += bin_size;
  }
  fclose (specout);

  //  Free allocated memory
  free (histo);

  return EXIT_SUCCESS;

}


