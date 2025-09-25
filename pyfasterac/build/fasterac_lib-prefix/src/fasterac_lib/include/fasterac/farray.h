
//
//
//  F A R R A Y
//
//  Array handling of faster data (faster_data_p*)
//
//



#ifndef FARRAY_H
#define FARRAY_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "fasterac/fasterac.h"


//---  farray structure  -------------------------------------------------//

typedef struct farray {
  //  memory space
  char*              data_space;
  size_t             space_size;
  //  data array
  faster_data_p*     data_array;
  int                nb_data;
  //  time width
  unsigned long long first_ns;
  unsigned long long last_ns;
} farray;

//---  memory management   -----------------------------------------------//

int farray_data_file_to_memory (const char* filename, char** mem_space, size_t* mem_size);
  //
  //  Allocate memory space and dump the file to that memory.
  //  Return the size of the memory space and a pointer to it (mem_space, mem_size).
  //  Return code : O on success, 1 on file error and 2 on memory error.
  //
  //  WARNING : the caller of that function has the RESPONSABILITY of the
  //            allocated memory (ie deallocation).
  //

int farray_data_file_to_memory_from_stdin (FILE* input, size_t size_max, char** mem_space, size_t* mem_size);
  //
  //  Allocate memory space and read the memory from the input.
  //  Return the size of the memory space and a pointer to it (mem_space, mem_size).
  //  Return code : O on success, 1 on file error and 2 on memory error.
  //
  //  WARNING : the caller of that function has the RESPONSABILITY of the
  //            allocated memory (ie deallocation).
  //

farray* farray_new (char* data_space, size_t space_size);
  //  Create new 'farray' with 'data_space'.
  //

void farray_free (farray* far);
  //  Free allocated memory without freeing the data space.
  //

//---  faster data array  ------------------------------------------------//

int farray_previous_idx (const farray far, unsigned long long clock_ns);
  //  Returns the index number of the oldest data before 'clock_ns'.
  //  Returns -1 on error;

int farray_next_idx (const farray far, unsigned long long clock_ns);
  //  Returns the index number of the youngest data after 'clock_ns'.
  //  Returns -1 on error;

int farray_nearest_idx (const farray far, unsigned long long clock_ns);
  //  Returns the index number of the nearest data to 'clock_ns'.
  //  Returns -1 on error;

#ifdef __cplusplus
}
#endif


#endif  // FARRAY_H
