
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>

#include "fasterac/fasterac.h"
#include "fasterac/fast_data.h"



//-------------------------------------------------------//
//         Oscillo data to ascii convertion
//-------------------------------------------------------//

void osc_ascii_file (oscillo        o,
                     short          nb_pts,
                     unsigned int   osc_num,
                     unsigned short label,
                     char*          output_dir) {
  //  Create an ascii XY file from a oscillo faster data
  //  units : X (ns) Y (mV)
  FILE*          fout;
  char           filename[256];
  int            point_num;
  float          x;
  float          y;
  float          toNanos = 1.0;
  float          toMV    = 1.0;
  sprintf (filename, "%s/osc%d_%d.dat", output_dir, osc_num, label);  //  output filename format
  fout  = fopen (filename, "w");
  if      (strcmp (o.xcap, "ns"     ) == 0) toNanos = 1.0;  //  x unit
  else if (strcmp (o.xcap, "ns     ") == 0) toNanos = 1.0;
  else if (strcmp (o.xcap, "us"     ) == 0) toNanos = 1000.0;
  else if (strcmp (o.xcap, "us     ") == 0) toNanos = 1000.0;
  else if (strcmp (o.xcap, "ms"     ) == 0) toNanos = 1000000.0;
  else if (strcmp (o.xcap, "ms     ") == 0) toNanos = 1000000.0;
  else printf ("WARNING : unknown time unit '%s'.\n", o.xcap);
  if      (strcmp (o.ycap, "mV"     ) == 0) toMV    = 1.0;  //  y unit
  else if (strcmp (o.ycap, "mV     ") == 0) toMV    = 1.0;
  else if (strcmp (o.ycap, "AVG mV" ) == 0) toMV    = 1.0;
  else if (strcmp (o.ycap, "AVG mV ") == 0) toMV    = 1.0;
  else if (strcmp (o.ycap, "V"      ) == 0) toMV    = 1000.0;
  else if (strcmp (o.ycap, "V      ") == 0) toMV    = 1000.0;
  else if (strcmp (o.ycap, "AVG V"  ) == 0) toMV    = 1000.0;
  else if (strcmp (o.ycap, "AVG V  ") == 0) toMV    = 1000.0;
  else printf ("WARNING : unknown Y unit '%s'.\n", o.ycap);
  for (point_num=0; point_num<nb_pts; point_num++) {
    x = (o.x0 + point_num  * o.xlsb) * toNanos;
    y = (o.samp[point_num] * o.ylsb) * toMV;
    fprintf (fout, "%f %f\n", x, y);                                 //  ascii data format
  }
  fclose (fout);
}

//-------------------------------------------------------//

void display_usage (char* prog_name) {
    printf ("\n");
    printf ("***  %s  *** \n", prog_name);
    printf ("\n");
    printf ("Convert faster oscillos to ascii files.\n");
    printf ("\n");
    printf ("usage : \n");
    printf ("       %s  input_file.fast  output_directory  [MAX_NB_FILES]\n", prog_name);
    printf ("\n");
    printf ("example : \n");
    printf ("          %s  /usr/share/fasterac/data/oscillo14_10k.fast  OSCIDIR\n", prog_name);
    printf ("\n");
    printf ("Each oscillo found in 'input_file.fast' is dumped to 'output_directory' as an ascii file.\n");
    printf ("The number of output files can be limited by MAX_NB_FILES.\n");
    printf ("The ascii format is two columns X(ns) Y(mV).\n");
    printf ("\n");
    printf ("\n");
}

//-------------------------------------------------------//

int main (int argc, char** argv) {

  faster_file_reader_p reader;                                //  data file reader
  faster_data_p        data;                                  //  data pointer
  char                 prog_name  [256];
  char                 output_dir [256];
  char                 input_file [256];
  unsigned short       label;
  oscillo              o;
  short                nb_pts;
  int                  nb_max = INT_MAX;
  int                  n = 0;

  strcpy (prog_name,  argv[0]);                                //  prog_name

  if (argc < 3) {                                              //  command args & usage
    display_usage (prog_name);
    return EXIT_FAILURE;
  }

  if (argc > 3) {
    nb_max = atoi (argv[3]);                                   //  max number of output files
  }

  strcpy (input_file, argv[1]);                                //  output_dir
  strcpy (output_dir, argv[2]);                                //  input_file

  if (mkdir (output_dir, S_IRWXU) != 0) {                      //  create output_dir
    printf ("ERROR creating directory %s\n", output_dir);
    display_usage (prog_name);
    return EXIT_FAILURE;
  }

  reader = faster_file_reader_open (input_file);               //  create a reader
  if (reader == NULL) {
    printf ("ERROR opening file %s\n", input_file);
    display_usage (prog_name);
    return EXIT_FAILURE;
  }

  while ((n < nb_max) &&                                       //  loop on data
         (data = faster_file_reader_next (reader)) != NULL) {
    if (faster_data_type_alias (data) == SAMPLING_TYPE_ALIAS) {//  is oscillo ?
      label = faster_data_label (data);                        //  get label
      n = n + 1;                                               //  incr osc num
      faster_data_load (data, &o);                             //  get oscillo part of data
      nb_pts = faster_data_oscillo_nb_pts (data);
      osc_ascii_file   (o, nb_pts, n, label, output_dir);      //  output sampling to ascii file
    }
  }
  faster_file_reader_close (reader);                           //  close the reader
  return EXIT_SUCCESS;
}


