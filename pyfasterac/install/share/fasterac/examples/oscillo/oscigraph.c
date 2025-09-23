
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "gnuplot_i.h"
#include "fasterac/fasterac.h"
#include "fasterac/fast_data.h"


//-------------------------------------------------------//

void usage_exit (char* prog) {
   fprintf (stderr, "\n");
   fprintf (stderr, "  Gnuplot oscillo from faster file\n");
   fprintf (stderr, "\n");
   fprintf (stderr, "  Usage: %s [-d delay_s] [-n n_max]  input_file.fast  osc_label\n", prog);
   fprintf (stderr, "\n");
   fprintf (stderr, "        input_file.fast : data file containing oscillos.\n");
   fprintf (stderr, "        osc_label       : selected oscillo.\n");
   fprintf (stderr, "        delay_s         : delay between two oscillos [default=asap].\n");
   fprintf (stderr, "        n_max           : quit after n_max oscillos  [default=all].\n");
   fprintf (stderr, "\n");
   exit    (EXIT_FAILURE);
}

//-------------------------------------------------------//
      
void osci_out (gnuplot_ctrl* graph, faster_data_p osci_data, char* title, double* min, double* max) {
   oscillo* osci;                        
   int      nb_pts;           
   int      n;
   double   x [OSCILLO_NB_PTS_MAX];
   double   y [OSCILLO_NB_PTS_MAX];
   int      up_range = 0;
   char     yrange [256];
   
   osci   = (oscillo*) faster_data_load_p (osci_data);                        
   nb_pts = faster_data_oscillo_nb_pts    (osci_data);            
   for (n=0; n<nb_pts; n++) {
      x [n] = osci->x0 + n * osci->xlsb;
      y [n] = osci->samp [n] * osci->ylsb;
      if (y [n] < *min) {
         *min = y [n];
         up_range = 1;
      } else if (y [n] > *max) {
         *max = y [n];
         up_range = 1;
      }
   }
   if (up_range) {
      sprintf (yrange, "set yrange [%f:%f]", *min, *max); 
      gnuplot_cmd (graph, yrange);
   }
   gnuplot_resetplot (graph);
   gnuplot_plot_xy   (graph, x, y, nb_pts, title);
}

//-------------------------------------------------------//

int main (int argc, char** argv) {

   //  cmd line
   extern char*   optarg;
   extern int     optind;
   int            opt;
   char*          prog_name = argv [0];
   char*          input_file;
   unsigned short label;
   float          delay = 0.0;
   unsigned int   usec  = 0;
   int            n_max = -1;
   //  faster file
   faster_file_reader_p reader;                                
   faster_data_p        data;                                  
   oscillo*             osci;
   int                  n = 0;
   //  gnuplot
   gnuplot_ctrl* graph;
   char          title  [256];
   char          xlab   [256];
   char          ylab   [256];
   double        min   = 1e6;
   double        max   = -1e6;

   while ((opt = getopt (argc, argv, "?hd:n:")) != -1) {
      switch (opt) {
         case 'd' :
            delay = atof (optarg);
            usec  = 1000000 * delay;
            break;
         case 'n' :
            n_max = atoi (optarg);
            break;
         case 'h' :
         case '?' :
         default  :
            usage_exit (prog_name);
      }
   }
   
   if (argc - optind < 2) usage_exit (prog_name);
   input_file =       argv [optind];
   label      = atoi (argv [optind + 1]);

   reader = faster_file_reader_open (input_file);               
   if (reader == NULL) {
      printf ("ERROR opening file %s\n", input_file);
      usage_exit (prog_name);
      return EXIT_FAILURE;
   }
   
   while ((data = faster_file_reader_next (reader)) != NULL) {  
      if (faster_data_type_alias (data) == OSCILLO_TYPE_ALIAS) { 
         if (faster_data_label (data) == label) {                 
            n++;
            break;
         }
      }
   }
   if (n) {
      graph = gnuplot_init ();
      osci  = faster_data_load_p (data);                        
      sprintf (xlab, "time %s",    osci->xcap);
      sprintf (ylab, "voltage %s", osci->ycap);
      sprintf (title, "%s oscillo %d", input_file, label);
      gnuplot_setstyle   (graph, "lines");
      gnuplot_set_xlabel (graph, xlab);
      gnuplot_set_ylabel (graph, ylab);
      osci_out (graph, data, title, &min, &max);
      usleep (usec);
      while ((data = faster_file_reader_next (reader)) != NULL) {
         if ((n_max >= 0) && (n >= n_max)) break;
         if (faster_data_type_alias (data) == OSCILLO_TYPE_ALIAS) { 
            if (faster_data_label (data) == label) {             
               n++;
               osci_out (graph, data, title, &min, &max);
               usleep (usec);
            }
         }
      }
      gnuplot_close (graph);
   }
   faster_file_reader_close (reader);                           


   return EXIT_SUCCESS;
}


