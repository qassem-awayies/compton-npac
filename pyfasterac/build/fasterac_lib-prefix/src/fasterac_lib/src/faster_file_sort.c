/*
 *  'faster_file_sort.c'
 *
 *  Convert an unsorted data file to a sorted one.
 *
 */



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fasterac/fasterac.h"       //  generic data, file reader and writer
#include "fasterac/farray.h"           //  faster file in memory


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
 *  Main prog
 */

int main (int argc, char** argv) {
  char*                data_space;                           //  file in memory
  size_t               space_size;
  int                  error;
  farray               *far;                                 //  faster data array
  faster_file_writer_p writer;                               //  file writer
  FILE*                f;                                    //  output file
  int                  i;

  if (argc < 3) {                                            //  command args & usage
    printf ("\nusage : \n");
    printf ("        %s  input_unsorted.fast   output_sorted.fast\n", argv[0]);
    printf ("\n");
    return EXIT_SUCCESS;
  }

  error = farray_data_file_to_memory (argv [1], &data_space, &space_size);
  if (error) {                                               // file to memory
    printf ("error opening %s\n", argv [1]);
    return 1;
  }
  far = farray_new (data_space, space_size);                 //  create far with data_space
  writer = faster_file_writer_open (argv [2]);               //  create a writer
  qsort (far->data_array,
         far->nb_data,
         sizeof (faster_data_p*),
         compare_data_clocks);                               //  sort the array (man qsort)
  for (i=0; i < far->nb_data; i++) {                         //  loop on data
    faster_file_writer_next (writer, far->data_array [i]);   //  put the array to output file
  }
  free                     (data_space);                     //  free allocated memory
  farray_free              (far);
  faster_file_writer_close (writer);                         //  close the writer
  return EXIT_SUCCESS;
}


