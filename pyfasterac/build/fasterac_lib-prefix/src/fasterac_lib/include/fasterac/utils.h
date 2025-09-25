
//
//  UTILITY FUNCTIONS definitions
//



#ifndef FASTER_UTILS_H
#define FASTER_UTILS_H 1

#ifdef __cplusplus
extern "C" {
#endif


#include "fasterac/fasterac.h"




///////////////////////////////////////////////////////////////////////
//
//  DATA TYPE NAMES
//
//

const char*          type_name (unsigned char type_alias);
const unsigned short type_size (unsigned char type_alias);


///////////////////////////////////////////////////////////////////////
//
//  TIME UNITS
//
//

typedef enum {
  TUNIT_NS = 0,
  TUNIT_US = 1,
  TUNIT_MS = 2,
  TUNIT_S  = 3
} time_unit;

const char tunit_str  [4][3] = {"ns", "us", "ms", "s"};
const int  tunit_coef [4]    = {1, 1000, 1000000, 1000000000};


////////////////////////////////////////////////////////////////////////
//
//  DATA DISPLAY
//
//

void data_display (faster_data_p data, int n, int tab, int full);
//  print a faster data in console
//  n    : data number
//  tab  : number of tabs added at beginning
//  full : data info (0=reduced, 1=full)

void relative_data_display (faster_data_p data, int n, int tab, int full, unsigned long long ref_ns, time_unit tunit);
//  same display as 'data_display' + relative clock ref (ns) and time unit selection
//



////////////////////////////////////////////////////////////////////////
//
//  HR_CLOCKS  (tdc data)
//
//

long double faster_data_hr_clock_sec (const faster_data_p data);
  // HR clock of data in second with 2/256 ns resolution.
  // For data without 'tdc', return the standard 2ns resolution time stamp.

long double faster_data_hr_clock_ns (const faster_data_p data);
  // HR clock of data in nanosec (without 'tdc' => standard time stamp)

long double faster_data_rf_time_ns (const faster_data_p data, const faster_data_p rf_ref);
  // Return the elapsed time between the data and the last RF front (calculated with the last rf data 'rf_ref').


#ifdef __cplusplus
}
#endif


#endif  // FASTER_UTILS_H
