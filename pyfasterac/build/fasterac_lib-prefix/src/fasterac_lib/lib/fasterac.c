#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#include "fasterac/fasterac.h"

extern const unsigned short type_size (unsigned char type_alias);

#define FASTER_TICK_SECOND 2.0e-9
#define FASTER_TICK_NS     2

static const char FASTER_MAGIC = 0xAA;

typedef enum {
  FASTER_DATA_HEADER_BYTE_SIZE   = 12,
  FASTER_DATA_LOAD_MAX_BYTE_SIZE = 8192,
  FASTER_CLOCK_BYTE_SIZE         = 6
} faster_const;


//  PACKAGE INFO

void fasterac_version_number (char* info) {
  strcpy (info, PACKAGE_VERSION);
}

void fasterac_verbose_version (char* info) {
 //sprintf (info, "%s  (%s)    %s    %s", PACKAGE_STRING, SVN_VERSION, PACKAGE_URL, PACKAGE_BUGREPORT);
 sprintf (info, "%s  (%s)    %s    %s", PACKAGE_STRING, GIT_TAG, PACKAGE_URL, PACKAGE_BUGREPORT);
}



//  DATA

typedef struct faster_data_header_t {
  unsigned char  type_alias;
  unsigned char  magic;
  unsigned char  clock [FASTER_CLOCK_BYTE_SIZE];
  unsigned short label;
  unsigned short load_size;
} faster_data_header_t;


typedef struct faster_data_t {
  faster_data_header_t  header;
  char                  load [FASTER_DATA_LOAD_MAX_BYTE_SIZE];
} faster_data_t;


unsigned char faster_data_type_alias (const faster_data_p data) {
  faster_data_t *d = (faster_data_t*) data;
  return d->header.type_alias;
}


unsigned short faster_data_label (const faster_data_p data) {
  faster_data_t *d = (faster_data_t*) data;
  return d->header.label;
}

unsigned short faster_data_set_label (const faster_data_p data, unsigned short label) {
  faster_data_t *d = (faster_data_t*) data;
  d->header.label = label;
  return 0;
}

long double faster_data_clock_sec (const faster_data_p data) {
  return ((long double) faster_data_clock_ns (data)) * 1e-9L;
}

unsigned long long faster_data_clock_ns (const faster_data_p data) {   // faster clock    48bits
  return ((*((unsigned long long*) data)) >> 16) * FASTER_TICK_NS;     // long long       64bits
}

unsigned short faster_data_set_clock_ns (const faster_data_p data, unsigned long long clock_ns) {
  faster_data_t *d = (faster_data_t*) data;
  char*  clock_p   = (char*)&(clock_ns);
  clock_ns         = clock_ns / FASTER_TICK_NS;
  memcpy (d->header.clock, clock_p, FASTER_CLOCK_BYTE_SIZE);
  return 0;
}

unsigned long long faster_data_add_clock_ns (const faster_data_p data, unsigned long long clock2add_ns) {
  faster_data_t*     d         = (faster_data_t*) data;
  unsigned long long new_clock = faster_data_clock_ns (data) + clock2add_ns;
  faster_data_set_clock_ns (data, new_clock);
  return new_clock;
}
unsigned short faster_data_load_size (const faster_data_p data) {
  faster_data_t *d = (faster_data_t*) data;
  return d->header.load_size;
}


unsigned short faster_data_set_load_size (const faster_data_p data, unsigned short load_size) {
  faster_data_t *d = (faster_data_t*) data;
  d->header.load_size = load_size;
  return 0;
}


unsigned short faster_data_load (const faster_data_p data, void* mem2cpy) {
  faster_data_t *d = (faster_data_t*) data;
  memcpy(mem2cpy, &d->load, d->header.load_size);
  return d->header.load_size;
}


void* faster_data_load_p (const faster_data_p data) {
  return (void*) ((char*) data + FASTER_DATA_HEADER_BYTE_SIZE);
}

faster_data_p faster_data_clone (const faster_data_p to_clone) {
  faster_data_t  *d1;
  faster_data_t  *d2;
  unsigned short size;
  d1   = (faster_data_t*) to_clone;
  size = FASTER_DATA_HEADER_BYTE_SIZE + d1->header.load_size;
  d2   = malloc (size);
  memcpy(d2, d1, size);
  return d2;
}


faster_data_p faster_data_new (const unsigned short label,
                               const unsigned short type_alias,
                               const unsigned long long clock_ns) {
  faster_data_t*     d1;
  unsigned short     size;
  unsigned short     load_size;
  char*              clock_p;
  unsigned long long clock;

  load_size             = type_size (type_alias);
  size                  = FASTER_DATA_HEADER_BYTE_SIZE + load_size;
  d1                    = malloc (size);
  d1->header.type_alias = type_alias;
  d1->header.magic      = FASTER_MAGIC;
  d1->header.label      = label;
  d1->header.load_size  = load_size;
  clock                 = clock_ns / FASTER_TICK_NS;
  clock_p               = (char*)&(clock);
  memcpy (d1->header.clock, clock_p, FASTER_CLOCK_BYTE_SIZE);

  return d1;
}

//  BUFFER

typedef struct faster_buffer_reader_t {
  unsigned char *buf;
  void          *buf_out;
  faster_data_t *current;
  void          *next;
} faster_buffer_reader_t;


faster_buffer_reader_p faster_buffer_reader_open (const void   *buffer,
                                                  const size_t  size) {
  faster_buffer_reader_t *fbr;
  fbr           = (faster_buffer_reader_t*) malloc (sizeof (faster_buffer_reader_t));
  fbr->buf      = (unsigned char*) buffer;
  fbr->buf_out  = fbr->buf + size;
  if (size < 12) {
    fbr->current = NULL;
    fbr->next    = fbr->buf_out;
  } else {
    fbr->current = (faster_data_t*) fbr->buf;
    fbr->next    = fbr->buf;
  }
  return fbr;
}


void faster_buffer_reader_close (faster_buffer_reader_p reader) {
  faster_buffer_reader_t *fbr = (faster_buffer_reader_t*) reader;
  free (fbr);
}


faster_data_p faster_buffer_reader_next (faster_buffer_reader_p reader) {
  faster_buffer_reader_t *fbr = (faster_buffer_reader_t*) reader;
  if (fbr->next >= fbr->buf_out) {
    return NULL;
  }
  fbr->current = fbr->next;
  fbr->next    = (char*)fbr->current
                 + sizeof (faster_data_header_t)
                 + faster_data_load_size (fbr->current);
  return (faster_data_p) fbr->current;
}


void* faster_buffer_reader_current_position (const faster_buffer_reader_p reader) {
  faster_buffer_reader_t *fbr = (faster_buffer_reader_t*) reader;
  return (void*) fbr->current;
}


//  FILE READER

typedef struct faster_file_reader_t {
  faster_data_t *current;
  gzFile           file;
} faster_file_reader_t;


faster_file_reader_p faster_file_reader_open (const char *filename) {
  faster_file_reader_t *ffr;
  ffr       = (faster_file_reader_t*) malloc (sizeof (faster_file_reader_t));
  ffr->file = gzopen (filename, "r");
  if (ffr->file == NULL) {
    free (ffr);
    return NULL;
  }
  ffr->current = (faster_data_t*) malloc (sizeof (faster_data_t));
  return ffr;
}


void  faster_file_reader_close (faster_file_reader_p reader) {
  faster_file_reader_t *ffr = (faster_file_reader_t*) reader;

  if (reader) {
    if (ffr->current != NULL) free (ffr->current);
    if (ffr->file    != NULL) gzclose (ffr->file);
    free (ffr);
    reader = NULL;
  }
}


faster_data_p faster_file_reader_next (faster_file_reader_p reader) {
  faster_file_reader_t *ffr = (faster_file_reader_t*) reader;
  int n;
  if (!gzeof (ffr->file)) {
    n = gzread (ffr->file, &ffr->current->header, sizeof (faster_data_header_t));
    if (n == sizeof (faster_data_header_t)) {
      // read header ok
      if (ffr->current->header.load_size == 0) {
        // no load is ok
        return ffr->current;
      } else {
        n = gzread (ffr->file, &ffr->current->load, ffr->current->header.load_size);
        if (n == ffr->current->header.load_size) {
          // load read ok
          return ffr->current;
        }
      }
    }
  }
  return NULL;
}


//  FILE WRITER

typedef struct faster_file_writer_t {
  FILE *file;
} faster_file_writer_t;


faster_file_writer_p  faster_file_writer_open (const char *filename) {
  faster_file_writer_t *ffw;
  ffw       = (faster_file_writer_t*) malloc (sizeof (faster_file_writer_t));
  ffw->file = fopen (filename, "w");
  if (ffw->file == NULL) {
    free (ffw);
    return NULL;
  }
  return ffw;
}


void faster_file_writer_close (faster_file_writer_p writer) {
  faster_file_writer_t *ffw = (faster_file_writer_t*) writer;
  if (writer) {
    if (ffw->file != NULL) fclose (ffw->file);
    free (ffw);
    writer = NULL;
  }
}


void faster_file_writer_next (const faster_file_writer_p writer, const faster_data_p data) {
  faster_file_writer_t *ffw    = (faster_file_writer_t*) writer;
  size_t                width  = sizeof (faster_data_header_t) + faster_data_load_size (data);
  fwrite (data, width, 1, ffw->file);
}







