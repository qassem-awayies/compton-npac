
#include <stdio.h>
#include <string.h>

#include "fasterac/fasterac.h"
#include "fasterac/fast_data.h"


//-------------------------------------------------------//

void display_usage (char* prog_name) {
    printf ("\n");
    printf ("  ***  %s  *** \n", prog_name);
    printf ("\n");
    printf ("  Output oscillo sum to an ascii file.");
    printf ("\n");
    printf ("  usage : \n");
    printf ("          %s  input_file.fast  osc_label  summation_file.wyw  [normalize_mVns=1000.0]\n", prog_name);
    printf ("\n");
    printf ("\n        input_file.fast    : data file containing oscillos.");
    printf ("\n        osc_label          : select oscillo channel.");
    printf ("\n        summation_file.wyw : resulting oscillo ascii file (two columns : Xns YmV).");
    printf ("\n        normalize_mVns     : oscillo total charge.");
    printf ("\n");
    printf ("  example : \n");
    printf ("          %s  /usr/share/fasterac/data/oscillo14_10k.fast  14  osc14.txt\n", prog_name);
    printf ("\n");
    printf ("\n");
}

//-------------------------------------------------------//

int main (int argc, char** argv) {

  faster_file_reader_p reader;                                //  data file reader
  faster_data_p        data;                                  //  data pointer
  char                 prog_name   [256];
  char                 input_file  [256];
  char                 output_file [256];
  FILE*                fout;
  unsigned short       label;
  double               normalize = 1000.0;
  double               charge;
  oscillo              osci;
  short                nb_pts;
  double               x;
  double               y [OSCILLO_NB_PTS_MAX] = {0.0};
  double               toNanos = 1.0;
  double               toMV    = 1.0;
  int                  n       = 0;

  strcpy (prog_name,  argv[0]);                                //  prog_name
  if (argc < 4) {                                              //  command args & usage
    display_usage (prog_name);
    return EXIT_FAILURE;
  }
  strcpy (input_file,  argv[1]);                               
  label = atoi        (argv[2]);                               
  strcpy (output_file, argv[3]);                               
  if (argc > 4) {
    normalize = atof  (argv[4]);                               
  }

  reader = faster_file_reader_open (input_file);               //  create a reader
  if (reader == NULL) {
    printf ("ERROR opening file %s\n", input_file);
    display_usage (prog_name);
    return EXIT_FAILURE;
  }
  while ((data = faster_file_reader_next (reader)) != NULL) {  //  loop on data
    if (faster_data_type_alias (data) == OSCILLO_TYPE_ALIAS) { //  is it oscillo ?
      if (faster_data_label (data) == label) {                 //  has the good label ?
        faster_data_load (data, &osci);                        //  get sampling part of data
        nb_pts = faster_data_oscillo_nb_pts (data);            //  get size of oscillo (number of points)
        for (n = 0; n < nb_pts; n++) {
          y [n]  += osci.samp [n];                             //  point summation
          charge += osci.samp [n];                             //  charge summation
        }
      }
    }
  }
  faster_file_reader_close (reader);                           //  close the reader

  if ((fout = fopen (output_file, "w")) == NULL) {             //  open output ascii file
    printf ("ERROR creating output file %s\n", output_file);
    display_usage (prog_name);
    return EXIT_FAILURE;
  }
  if      (strcmp (osci.xcap, "ns"     ) == 0) toNanos = 1.0;  //  x unit
  else if (strcmp (osci.xcap, "ns     ") == 0) toNanos = 1.0;
  else if (strcmp (osci.xcap, "us"     ) == 0) toNanos = 1000.0;
  else if (strcmp (osci.xcap, "us     ") == 0) toNanos = 1000.0;
  else if (strcmp (osci.xcap, "ms"     ) == 0) toNanos = 1000000.0;
  else if (strcmp (osci.xcap, "ms     ") == 0) toNanos = 1000000.0;
  else printf ("WARNING : unknown time unit '%s'.\n", osci.xcap);
  if      (strcmp (osci.ycap, "mV"     ) == 0) toMV    = 1.0;  //  y unit
  else if (strcmp (osci.ycap, "mV     ") == 0) toMV    = 1.0;
  else if (strcmp (osci.ycap, "AVG mV" ) == 0) toMV    = 1.0;
  else if (strcmp (osci.ycap, "AVG mV ") == 0) toMV    = 1.0;
  else if (strcmp (osci.ycap, "V"      ) == 0) toMV    = 1000.0;
  else if (strcmp (osci.ycap, "V      ") == 0) toMV    = 1000.0;
  else if (strcmp (osci.ycap, "AVG V"  ) == 0) toMV    = 1000.0;
  else if (strcmp (osci.ycap, "AVG V  ") == 0) toMV    = 1000.0;
  else printf ("WARNING : unknown Y unit '%s'.\n", osci.ycap);
  charge    = charge * osci.ylsb * toMV * osci.xlsb * toNanos; //  total charge (mVns)
  normalize = normalize / charge;                              //  normalization coef
  for (n = 0; n < nb_pts; n++) {
    x     = (osci.x0 + n * osci.xlsb) * toNanos;               //  x (ns)
    y [n] = y [n] * osci.ylsb * toMV;                          //  y (mV)
    y [n] = y [n] * normalize;                                 //  y (mV) with normalized total charge
    fprintf (fout, "%f %f\n", x, y [n]);                       //  two column ascii data format
  }
  fclose (fout);                                               //  close file

  return EXIT_SUCCESS;
}


