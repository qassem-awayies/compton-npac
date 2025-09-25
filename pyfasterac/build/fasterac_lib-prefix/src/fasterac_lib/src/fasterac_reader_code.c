/*
 *  Faster reader code writer
 *
 */

#include <stdio.h>
#include <string.h>

#include "fasterac/fasterac.h"
#include "fasterac/utils.h"
#include "fasterac/fast_data.h"
#include "fasterac/qdc.h"
#include "fasterac/group.h"
#include "fasterac/spectro.h"
#include "fasterac/jdb_hv.h"
#include "fasterac/qt2t.h"
#include "fasterac/rf.h"
#include "fasterac/sampler.h"
#include "fasterac/scaler.h"
#include "fasterac/qtdc.h"
#include "fasterac/electrometer.h"
#include "fasterac/plas.h"

void replace_str (char* out_str, const char* in_str, const char* replace, const char* by)
{
   const char*  where;
   const char*  str     = in_str;
   const size_t rep_len = strlen (replace);
   const size_t by_len  = strlen (by);
   size_t before_len;
   size_t len;
   size_t out_len = 0;
   while (where = strstr (str, replace))
   {
      len        = strlen (str);
      before_len = where - str;
      memcpy (out_str + out_len, str, before_len);
      out_len += before_len;
      memcpy (out_str + out_len, by, by_len);
      out_len += by_len;
      str     += before_len + rep_len;
   }
   strcpy (out_str + out_len, str);
}

void type_selection (faster_data_p data, int* selection) {

      unsigned char alias = faster_data_type_alias (data);

           if (alias == OSCILLO_TYPE_ALIAS)            selection  [0] = 1;
      else if (alias == TREF_TDC_TYPE_ALIAS)           selection  [1] = 1;
      else if (is_qdc (data))                          selection  [2] = 1;
      else if (alias == QDC_COUNTER_TYPE_ALIAS)        selection  [3] = 1;
      else if (alias == CRRC4_SPECTRO_TYPE_ALIAS)      selection  [4] = 1;
      else if (alias == TRAPEZ_SPECTRO_TYPE_ALIAS)     selection  [5] = 1;
      else if (alias == SPECTRO_COUNTER_TYPE_ALIAS)    selection  [6] = 1;
      else if (alias == JDB_HV_TYPE_ALIAS)             selection  [7] = 1;
      else if (alias == QT2T_TYPE_ALIAS)               selection  [8] = 1;
      else if (alias == QT2T_COUNTER_TYPE_ALIAS)       selection  [9] = 1;
      else if (alias == RF_DATA_TYPE_ALIAS)            selection [10] = 1;
      else if (alias == RF_COUNTER_TYPE_ALIAS)         selection [11] = 1;
      else if (alias == SAMPLER_DATA_TYPE_ALIAS)       selection [12] = 1;
      else if (alias == SAMPLER_COUNTER_TYPE_ALIAS)    selection [13] = 1;
      else if (alias == SCALER_MEASUREMENT_TYPE_ALIAS) selection [14] = 1;
      else if (alias == SCALER_COUNTER_TYPE_ALIAS)     selection [15] = 1;
      else if (alias == QTDC_TYPE_ALIAS)               selection [16] = 1;
      else if (alias == QTDC_COUNTER_TYPE_ALIAS)       selection [17] = 1;
      else if (alias == ELECTROMETER_TYPE_ALIAS)       selection [18] = 1;
      else if (alias == PLAS_RAW_DATA_TYPE_ALIAS)      selection [19] = 1;
      else if (alias == GROUP_COUNTER_TYPE_ALIAS)      selection [21] = 1;
      else if (alias == GROUP_TYPE_ALIAS){             selection [20] = 1;
         faster_data_p          group_data;
         unsigned short         lsize        = faster_data_load_size     (data);
         void*                  group_buffer = faster_data_load_p        (data);
         faster_buffer_reader_p group_reader = faster_buffer_reader_open (group_buffer, lsize);
         while ((group_data = faster_buffer_reader_next (group_reader)) != NULL) {
            type_selection (group_data, selection);
         }
         faster_buffer_reader_close (group_reader);
      }

}

int main (int argc, char** argv) {
   //  nb specific MnMs - code names and code includes
   int  nb_sel         = 22;
   char tname [22][10] = {{"@OSCILLO@"},
                          {"@TREF_T@" },
                          {"@QDCS@"   },
                          {"@QDC_C@"  },
                          {"@CRRC4@"  },
                          {"@TRAPEZ@" },
                          {"@ADC_C@"  },
                          {"@JDB_HV@" },
                          {"@QT2T@"   },
                          {"@QT2T_C@" },
                          {"@RF@"     },
                          {"@RF_C@"   },
                          {"@SAMPLER@"},
                          {"@SMPLR_C@"},
                          {"@SCALER@" },
                          {"@SCLR_C@" },
                          {"@QTDC@"   },
                          {"@QTDC_C@" },
                          {"@ELECTRO@"},
                          {"@PLAS@"   },
                          {"@GROUP@"  },
                          {"@GROUP_C@"}};
   char tinc  [22][13] = {{"@FASTDATA_H@"},
                          {"@FASTDATA_H@"},
                          {"@QDC_H@"     },
                          {"@QDC_H@"     },
                          {"@SPECTRO_H@" },
                          {"@SPECTRO_H@" },
                          {"@SPECTRO_H@" },
                          {"@JDB_H@"     },
                          {"@QT2T_H@"    },
                          {"@QT2T_H@"    },
                          {"@RF_H@"      },
                          {"@RF_H@"      },
                          {"@SAMPLER_H@" },
                          {"@SAMPLER_H@" },
                          {"@SCALER_H@"  },
                          {"@SCALER_H@"  },
                          {"@QTDC_H@"    },
                          {"@QTDC_H@"    },
                          {"@ELECTRO_H@" },
                          {"@PLAS_H@"    },
                          {"@GROUP_H@"   },
                          {"@GROUP_H@"   }};
   int                   selection [nb_sel];
   faster_file_reader_p  reader;
   faster_data_p         data;
   FILE*                 code_in;
   FILE*                 code_out;
   char                  line_in  [256];
   char                  line_out [256];
   char                  make_in  [4096];
   char                  codename [256];
   char                  c_name   [300];
   char                  m_name   [300];
   int                   line_ok;
   int                   makesize;
   int                   i;
   if (argc < 3) {
      printf ("\n");
      printf ("\n");
      printf ("The program %s is intended to people who want to write their own faster data reader.\n", argv [0]);
      printf ("This program generates a reader example code for a given faster data file.\n");
      printf ("The resulting code will show how to handle all the data of that file.\n");
      printf ("\n");
      printf ("usage : \n");
      printf ("        %s  my_faster_file.fast  my_reader\n", argv[0]);
      printf ("\n");
      printf ("        generates my_reader.c and my_reader.make\n");
      printf ("\n");
      return EXIT_SUCCESS;
   }
   //  Exec, Code, Makefile names
   sprintf (codename, "%s",      argv [2]);
   sprintf (c_name,   "%s.c",    codename);
   sprintf (m_name,   "%s.make", codename);
   //  Get type selection from data file
   reader = faster_file_reader_open (argv[1]);
   if (reader == NULL) {
      printf ("error opening file %s\n", argv[1]);
      return EXIT_FAILURE;
   }
   for (i=0; i<nb_sel; i++) selection [i] = 0;
   while ((data = faster_file_reader_next (reader)) != NULL) {
      type_selection (data, selection);
   }
   faster_file_reader_close (reader);
   //  Get code = f (selection) => write specific code
   code_in  = fopen ("/Users/qassem.awayies/Projects/compton-npac/pyfasterac/install/share/fasterac/src/dmo/faster_reader_demo_code.c.in", "r");
   code_out = fopen (c_name, "w");
   fprintf (code_out, "//\n");
   fprintf (code_out, "// code generated by the command : %s %s %s\n", argv [0], argv [1], argv [2]);
   fprintf (code_out, "//\n");
   fprintf (code_out, "//\n");
   while (fgets (line_in, 256, code_in) != NULL) {
      line_ok = 0;
      if (strstr (line_in, "@ANY@") != NULL) {
         line_ok = 1;
      } else {
         for (i=0; i<nb_sel; i++) {
            if (selection [i]) {
               if ((strstr (line_in, tname [i]) != NULL) ||
                   (strstr (line_in, tinc  [i]) != NULL))  {
                  line_ok = 1;
                  break;
               }
            }
         }
      }
      if (line_ok) {
         strcpy (line_out, &line_in [14]);
         fputs  (line_out, code_out);
      }
   }
   fclose  (code_out);
   fclose  (code_in);
   //  Write specific makefile
   code_in  = fopen ("/Users/qassem.awayies/Projects/compton-npac/pyfasterac/install/share/fasterac/src/dmo/autoreader.make.in", "r");
   code_out = fopen (m_name, "w");
   fprintf (code_out, "#\n");
   fprintf (code_out, "# code generated by the command : %s %s %s\n", argv [0], argv [1], argv [2]);
   fprintf (code_out, "#\n");
   fprintf (code_out, "#\n");
   strcpy (line_in, "");
   while (fgets (line_in, 256, code_in) != NULL) {
      replace_str (line_out, line_in, "@reader@", codename);
      fprintf (code_out, "%s", line_out);
   }
   fclose  (code_out);
   fclose  (code_in);
   //  Output infos
   printf  ("\n");
   printf  ("\n");
   printf  ("  output files : %s & %s\n", c_name, m_name);
   printf  ("\n");
   printf  ("  now read %s and try the following commands :\n", c_name);
   printf  ("\n");
   printf  ("    > make -f %s\n", m_name);
   printf  ("    > ./%s %s\n", codename, argv [1]);
   printf  ("\n");
   printf  ("\n");
   return EXIT_SUCCESS;
}

