
//
//  Scaler CARAS data definitions
//



#ifndef SCALER_H
#define SCALER_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>

#include "fasterac/fasterac.h"


//  CONST

typedef enum {
  SCALER_MEASUREMENT_TYPE_ALIAS =  82,
  SCALER_COUNTER_TYPE_ALIAS     =  83
} scaler_const;

static const char* SCALER_MEASUREMENT_TYPE_NAME = "SCALER_MEAS";
static const char* SCALER_COUNTER_TYPE_NAME     = "SCALER_COUNT";


//  DATA DEFINITIONS

typedef struct scaler_measurement {
  signed   max_ampl  : 20;    //  maximum amplitude
  unsigned nib       :  7;
  unsigned n_quanta  :  4;    //  number of quanta
  unsigned saturated :  1;    //  saturated signal
  unsigned fw_thres  : 16;    //  full width at threshold
  unsigned max_pos   : 16;    //  position of max
  signed   qtt       : 32;    //  charge auto thres2thres
} scaler_measurement;


typedef struct scaler_counter {
  unsigned quanta : 32;
  unsigned calc   : 32;
  unsigned sent   : 32;
} scaler_counter;


static size_t SCALER_MEASUREMENT_TYPE_SIZE = sizeof (scaler_measurement);
static size_t SCALER_COUNTER_TYPE_SIZE     = sizeof (scaler_counter);

// CONVERSION ACCESSORS

double scaler_max_ampl_mV (scaler_measurement scaler);
double scaler_max_pos_ns  (scaler_measurement scaler);
double scaler_qtt_mVns    (scaler_measurement scaler);
double scaler_fw_thres_ns (scaler_measurement scaler);


//  DATA TO STRING (used by faster_disfast)

static inline void scaler_measurement_attributes_str (faster_data_p data, char* scaler_str) {
   scaler_measurement scaler_meas;
   faster_data_load (data, &scaler_meas);
   sprintf (scaler_str,
            "  max_ampl=%d  max_pos=%fns  n_quanta=%d  fw_thres=%fns  qtt=%d", scaler_meas.max_ampl,
                                                                               scaler_max_pos_ns  (scaler_meas),
                                                                               scaler_meas.n_quanta,
                                                                               scaler_fw_thres_ns (scaler_meas),
                                                                               scaler_meas.qtt);
   if (scaler_meas.saturated) sprintf (scaler_str, "%s  saturated", scaler_str);
}


static inline void scaler_counter_attributes_str (faster_data_p data, char* count_str) {
   scaler_counter count;
   faster_data_load (data, &count);
   sprintf (count_str, "  quanta=%d  calc=%d  sent=%d", count.quanta, count.calc, count.sent);
}


#ifdef __cplusplus
}
#endif


#endif  // SCALER_H
