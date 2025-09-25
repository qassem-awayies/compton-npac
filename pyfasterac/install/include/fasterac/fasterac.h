
//
//  Faster Analysis in C
//  Minimum library for dealing with files and data.
//



#ifndef FASTERAC_H
#define FASTERAC_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

//
//  PACKAGE INFO
//

void fasterac_version_number  (char* info);
void fasterac_verbose_version (char* info);


//
//  DATA
//

typedef void* faster_data_p;
  //  Pointer to a Faster data

unsigned char faster_data_type_alias (const faster_data_p data);
  //  Type_Alias of a data

unsigned short faster_data_label (const faster_data_p data);
  //  Label of a data

unsigned short faster_data_set_label (const faster_data_p data, unsigned short label);
  //  Label of a data

unsigned long long faster_data_clock_ns (const faster_data_p data);
  //  Clock of a data (in nanosecond)

long double faster_data_clock_sec (const faster_data_p data);
  //  Clock of a data (in second)

unsigned short faster_data_set_clock_ns (const faster_data_p data, unsigned long long clock_ns);
  //  Set new clock ns to the data clock (in ns)

unsigned long long faster_data_add_clock_ns (const faster_data_p data, unsigned long long time2add_ns);
  //  Added time2add_ns to the data clock (in ns)

unsigned short faster_data_load_size (const faster_data_p data);
  //  Load_Size in byte

unsigned short faster_data_set_load_size (const faster_data_p data, unsigned short load_size);
  //  Set Load_Size in byte

unsigned short faster_data_load (const faster_data_p data, void* mem2cpy);
  //  Copies the load part of a faster data in mem2pcy
  //  and returns the number of bytes copied
  //  (care : mem2cpy size >= data load_size)

void* faster_data_load_p (const faster_data_p data);
  //  Returns a pointer to the load part of a data

faster_data_p faster_data_clone (const faster_data_p to_clone);
  //  Allocates memory and creates a clone the one passed in argument.

faster_data_p faster_data_new (const unsigned short label, const unsigned short type_alias, const unsigned long long clock);
  //  Allocates memory and creates a new data of type_alias.

//
//  BUFFER READER
//

typedef void* faster_buffer_reader_p;
  //  Pointer to a faster data buffer

faster_buffer_reader_p faster_buffer_reader_open (const void* buf, const size_t size);
  //  Gets a new reader for buf

void faster_buffer_reader_close (faster_buffer_reader_p reader);
  //  Frees the reader

faster_data_p faster_buffer_reader_next (faster_buffer_reader_p reader);
  //  Gets the next data from the buffer
  //  Returns NULL at end of buffer

void* faster_buffer_reader_current_position (const faster_buffer_reader_p reader);
  //  Gets a pointer to the current position of the reader within the initial buffer


//
//  FILE READER
//

typedef void* faster_file_reader_p;
  //  Pointer to a faster file reader

faster_file_reader_p  faster_file_reader_open (const char *filename);
  //  Opens the file and returns a new reader for it

void  faster_file_reader_close (faster_file_reader_p reader);
  //  Closes the file and frees the reader

faster_data_p faster_file_reader_next (faster_file_reader_p reader);
  //  Increments the position in the file and returns a pointer to the next data
  //  (current value of that pointer won't be available after the next 'next')
  //  Returns null if eof


//
//  FILE WRITER
//

typedef void* faster_file_writer_p;
  //  Pointer to a faster file writer

faster_file_writer_p  faster_file_writer_open (const char *filename);
  //  Opens the file and returns a new writer for it

void faster_file_writer_close (faster_file_writer_p writer);
  //  Closes the file and frees the writer

void faster_file_writer_next (const faster_file_writer_p writer, const faster_data_p data);
  //  Write the data to the file


#ifdef __cplusplus
}
#endif


#endif  // FASTERAC_H
