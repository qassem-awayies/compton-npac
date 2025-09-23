
//
//  PLAS data definitions
//



#ifndef PLAS_H
#define PLAS_H 1

#ifdef __cplusplus
extern "C" {
#endif


#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "fasterac/fasterac.h"


//  CONST

typedef enum {
   PLAS_COUNTER_TYPE_ALIAS       =  24,
   PLAS_RAW_DATA_TYPE_ALIAS      =  25,
   PLAS_V1_DATA_TYPE_ALIAS       =  26,
   PLAS_V2_DATA_TYPE_ALIAS       =  27,
   PLAS_DATA_TYPE_ALIAS          =  28,
   PLAS_POST_TRIGGER_NUMBER      =  6,
   PLAS_POST_TRIGGER_SAMPLES     =  32,
   PLAS_PRE_TRIGGER_SAMPLES      =  31,
   PLAS_BASELINE_SAMPLES         =  6,
   PLAS_ANALOG_SAMPLES           =  PLAS_BASELINE_SAMPLES + PLAS_PRE_TRIGGER_SAMPLES + PLAS_POST_TRIGGER_SAMPLES,
} plas_const;

typedef enum {
   PLAS_RANGE_150MV   =  0,
   PLAS_RANGE_300MV   =  1,
   PLAS_RANGE_600MV   =  2,
   PLAS_RANGE_1200MV  =  3,
   PLAS_RANGE_2400MV  =  4
} plas_range_t;

static const char* PLAS_RAW_TYPE_NAME     = "PLAS_RAW";
static const char* PLAS_V1_TYPE_NAME      = "PLAS_V1";
static const char* PLAS_COUNTER_TYPE_NAME = "PLAS_COUNT";


//  DATA DEFINITIONS

// PLAS_RAW
typedef struct plas_digital {
  unsigned device_address:4;     // 4
  unsigned dummy:1;              // 5
  unsigned input_channel:5;      // 10
  unsigned start_position:4;     // 14
  unsigned baseline_position:3;  // 17
  unsigned trigger_source:1;     // 18
  unsigned output_channel:4;     // 22
  unsigned raw_timestamp_1:2;    // 24
  unsigned raw_timestamp_2:8;    // 32
  unsigned raw_timestamp_3:25;   // 57
  unsigned ecc:6;                // 63
  unsigned parity:1;             // 64
} plas_digital;


typedef struct pre_trigger {
  short dummy;
  short samples[PLAS_PRE_TRIGGER_SAMPLES];
} pre_trigger;

typedef struct baseline {
  short samples[PLAS_BASELINE_SAMPLES];
} baseline;

typedef struct post_trigger {
  short dummy;
  short first_dummy;
  short samples[PLAS_POST_TRIGGER_SAMPLES];
} post_trigger;

typedef struct plas_analog {
  pre_trigger  pre_trigger;
  baseline     baseline;
  post_trigger post_trigger[PLAS_POST_TRIGGER_NUMBER];
} plas_analog;

typedef struct plas_raw {
  plas_digital digital;
  plas_analog  analog;
} plas_raw;

short faster_data_plas_nb_pts (faster_data_p data);

typedef short* plas_data;

typedef struct plas_counter {
  unsigned w1 : 32;
} plas_counter;


static size_t PLAS_RAW_DATA_TYPE_SIZE = sizeof (plas_raw);

static inline unsigned short plas_raw_get_baseline_samples     (faster_data_p data, short buffer [PLAS_BASELINE_SAMPLES], bool sorted) {

  int i,j;
  plas_raw plas;

  faster_data_load (data, &plas);
  if (sorted == true) {
    j = 0;
    for (i=plas.digital.baseline_position; j < PLAS_BASELINE_SAMPLES; i = (i+1) % PLAS_BASELINE_SAMPLES) {
      buffer[j] = plas.analog.baseline.samples[i];
      j++;
    }
  }else{
    for (i=0; i < PLAS_BASELINE_SAMPLES; i++) {
      buffer[i] = plas.analog.baseline.samples[i];
    }
  }

  return PLAS_BASELINE_SAMPLES;
}

static inline unsigned short plas_raw_get_pre_trigger_samples  (faster_data_p data, short buffer [PLAS_PRE_TRIGGER_SAMPLES], bool sorted) {
  int i,j;
  plas_raw plas;
  faster_data_load (data, &plas);

  if (sorted == true) {
    j = 0;
    for (i = plas.digital.start_position*2; j < PLAS_PRE_TRIGGER_SAMPLES; i = (i+1) % PLAS_PRE_TRIGGER_SAMPLES) {
        buffer[j] = plas.analog.pre_trigger.samples[i];
        j++;
    }
  }else{
    for (i=0; i < PLAS_PRE_TRIGGER_SAMPLES; i++) {
        buffer[i] = plas.analog.pre_trigger.samples[i];
    }
  }
  return PLAS_PRE_TRIGGER_SAMPLES;
}

static inline unsigned short plas_raw_get_post_trigger_samples (faster_data_p data, short buffer [PLAS_POST_TRIGGER_NUMBER*PLAS_POST_TRIGGER_SAMPLES]) {
  int i,j,k;
  plas_raw plas;
  faster_data_load (data, &plas);

  k = 0;
  for (i=0; i < PLAS_POST_TRIGGER_NUMBER; i++){
    for (j=0; j < PLAS_POST_TRIGGER_SAMPLES; j++){
      buffer[k++] = plas.analog.post_trigger[i].samples[j];
    }
  }
  return k;
}

static inline unsigned short plas_raw_get_analog_samples (faster_data_p data, short buffer [PLAS_ANALOG_SAMPLES], bool sorted) {

  unsigned short read_samples;
  short*         p_buffer;

  p_buffer     = &buffer[0];
  read_samples = 0;
  read_samples  = plas_raw_get_baseline_samples     (data, p_buffer + read_samples, sorted);
  read_samples += plas_raw_get_pre_trigger_samples  (data, p_buffer + read_samples, sorted);
  read_samples += plas_raw_get_post_trigger_samples (data, p_buffer + read_samples);

  return read_samples;
}

//  DATA TO STRING (used by faster_disfast)
static inline void plas_raw_attributes_str (faster_data_p data, char* plas_str) {
  int i,j;
  plas_raw plas;
  faster_data_load (data, &plas);
  sprintf          (plas_str, "   device_address=%d input_channel=%d start_position=%2d baseline_position=%d trigger_source=%d output_channel=%d parity=%d ecc=%3d\n", plas.digital.device_address,
                                                                                                                                                                       plas.digital.input_channel,
                                                                                                                                                                       plas.digital.start_position,
                                                                                                                                                                       plas.digital.baseline_position,
                                                                                                                                                                       plas.digital.trigger_source,
                                                                                                                                                                       plas.digital.output_channel,
                                                                                                                                                                       plas.digital.parity,
                                                                                                                                                                       plas.digital.ecc);
  sprintf (plas_str, "   %sBaseline samples :\n ", plas_str);
  for (i=0; i < PLAS_BASELINE_SAMPLES; i++){
      sprintf (plas_str, " %s%d; ", plas_str, plas.analog.baseline.samples[i]);
  }
  sprintf (plas_str, "   %s\nPre  trigger samples :\n ", plas_str);
  for (i=0; i < PLAS_PRE_TRIGGER_SAMPLES; i++){
      sprintf (plas_str, " %s%d; ", plas_str, plas.analog.pre_trigger.samples[i]);
  }
  sprintf (plas_str, "   %s\nPost trigger samples (10):\n ", plas_str);
  for (i=0; i < 1; i++){
    for (j=0; j < 10; j++){
      sprintf (plas_str, " %s%d; ", plas_str, plas.analog.post_trigger[i].samples[j]);
    }
  }
}


static inline void plas_counter_attributes_str (faster_data_p data, char* count_str) {
   plas_counter count;
   faster_data_load (data, &count);
   sprintf          (count_str, "  TO DO");
}


#ifdef __cplusplus
}
#endif


#endif  // PLAS_H
