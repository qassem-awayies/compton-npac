
//
//  RF CARAS data definitions
//



#ifndef RF_H
#define RF_H 1

#ifdef __cplusplus
extern "C" {
#endif


#include <math.h>
#include <stdio.h>

#include "fasterac/fasterac.h"



//  CONST


typedef enum {
  RF_DATA_TYPE_ALIAS    = 19,
  RF_COUNTER_TYPE_ALIAS = 20,
} rf_const;

static const char* RF_DATA_TYPE_NAME    = "RF";
static const char* RF_COUNTER_TYPE_NAME = "RF COUNTER";


//  DATA DEFINITIONS

typedef struct rf_data {
  unsigned period    : 31;
  unsigned saturated :  1;
    signed trig_dt   : 32;
    signed pll_dt    : 32;
} rf_data;


typedef struct rf_counter {
  unsigned trig : 32;
  unsigned sent : 32;
} rf_counter;


static size_t RF_DATA_TYPE_SIZE    = sizeof (rf_data   );
static size_t RF_COUNTER_TYPE_SIZE = sizeof (rf_counter);

//  CONVERSION ACCESSORS

double rf_period_ns  (rf_data rf);
double rf_trig_dt_ns (rf_data rf);
double rf_pll_dt_ns  (rf_data rf);




//  DATA TO STRING (used by faster_disfast)

  static inline void rf_data_attributes_str (faster_data_p data, char* rf_str) {
     rf_data     rf;
     long double hr_clock;
     faster_data_load (data, &rf);
     sprintf (rf_str, "  period=%.6lfns", rf_period_ns (rf));
     hr_clock =  (long double) faster_data_clock_ns (data) + rf_trig_dt_ns (rf);
     sprintf (rf_str, "%s  raw_trig=%.3Lfns", rf_str, hr_clock);
     hr_clock =  (long double) faster_data_clock_ns (data) + rf_pll_dt_ns (rf);
     sprintf (rf_str, "%s  pll_trig=%.3Lfns", rf_str, hr_clock);
     if (rf.saturated) {
       sprintf (rf_str, "%s  saturated", rf_str);
     }
  }

  static inline void rf_counter_attributes_str (faster_data_p data, char* counter_str) {
     rf_counter counter;
     faster_data_load (data, &counter);
     sprintf (counter_str, "  trig=%d  sent=%d", counter.trig, counter.sent);
  }


#ifdef __cplusplus
}
#endif


#endif  // RF_H
