/*
 *  'faster_file_ungroup.c'
 *
 *  Convert a data file containing grouped data to a flattened and sorted one.
 *
 */



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fasterac/fasterac.h"       //  generic data, file reader and writer
#include "fasterac/group.h"          //  group data
#include "fasterac/farray.h"         //  file in memory


/*
 *  Comparison function for qsort (man qsort)
 *  Function prototype : int(*compar)(const void *, const void *)
 */

int compare_data_clocks (const void* data1, const void* data2) {
  faster_data_p *d1 = (faster_data_p*) data1;
  faster_data_p *d2 = (faster_data_p*) data2;
  long double dt;
  dt = faster_data_clock_sec (*d1) - faster_data_clock_sec (*d2);
  if (dt > 0) return  1;
  if (dt < 0) return -1;
  return 0;
}


/*
 *  Put a single data to dataseq[pos] and increment pos.
 *  When data is a group, ungroup_to_dataseq each grouped data.
 */

void ungroup_to_dataseq (faster_data_p data, faster_data_p* dataseq, int* pos) {
  unsigned char          alias = faster_data_type_alias (data);
  unsigned short         lsize = faster_data_load_size  (data);
  faster_buffer_reader_p group_reader;
  void*                  group_buffer;
  faster_data_p          group_data;
  if (alias != GROUP_TYPE_ALIAS) {
    dataseq [*pos] = data;
    *pos           = *pos + 1;
  } else {
    group_buffer = faster_data_load_p (data);
    group_reader = faster_buffer_reader_open (group_buffer, lsize);
    while ((group_data = faster_buffer_reader_next (group_reader)) != NULL) {
      ungroup_to_dataseq (group_data, dataseq, pos);
    }
    faster_buffer_reader_close (group_reader);
  }
}


/*
 *  Main prog
 */

int main (int argc, char** argv) {
  char*                data_space;                                                  //  file in memory
  size_t               space_size;
  int                  error;
  farray*              far;                                                         //  faster data array
  faster_file_writer_p out_writer;                                                  //  file writer
  FILE*                f;                                                           //  output file
  faster_data_p        data;                                                        //  a data
  faster_data_p*       ugrp_array;                                                  //  ungrouped and sorted resulting data array
  int                  n = 0;                                                       //  nb of data to write
  int                  i;

  if (argc < 3) {                                                                   //  command args & usage
    printf ("\nusage : \n");
    printf ("        %s  input_file_with_groups.fast   flattened_output_file.fast\n", argv[0]);
    printf ("\n");
    return EXIT_SUCCESS;
  }

  error = farray_data_file_to_memory (argv [1], &data_space, &space_size);            // file to memory
  if (error) {
    printf ("error opening %s\n", argv [1]);
    return 1;
  }
  far = farray_new (data_space, space_size);                                        //  create far with data_space
  out_writer = faster_file_writer_open (argv [2]);                                  //  create the output writer
  ugrp_array = (faster_data_p*) malloc (sizeof (faster_data_p) * space_size/12);    //  maximize the allocation for the ungrouped array
  for (i=0; i < far->nb_data; i++) {                                                //  loop on data
    ungroup_to_dataseq (far->data_array [i], ugrp_array, &n);                       //  append data to ugrp_array and increment n
  }                                                                                 //  (ungroup if needed, recursive)
  ugrp_array = (faster_data_p*) realloc (ugrp_array, sizeof (faster_data_p) * n);   //  adjust the sequence array size
  qsort (ugrp_array, n, sizeof (faster_data_p*), compare_data_clocks);              //  sort the array (man qsort)
  for (i=0; i<n; i++) {                                                             //  loop on ugrp_array
    if (faster_data_type_alias (ugrp_array[i]) != GROUP_COUNTER_TYPE_ALIAS) {
      faster_file_writer_next (out_writer, ugrp_array [i]);                       //  output data to file
    }
  }                                                                                 //
  faster_file_writer_close (out_writer);                                            //  close the writer
  farray_free              (far);
  free                     (data_space);                                            //  free allocated memory
  free                     (ugrp_array);
  return EXIT_SUCCESS;

}


