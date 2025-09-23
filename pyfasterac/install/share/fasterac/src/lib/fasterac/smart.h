
//
//  SAMRT data definitions
//



#ifndef SMART_H
#define SMART_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>

#include "fasterac/fasterac.h"


typedef enum {
  SMART_TYPE_ALIAS     =  5,
} smart_const;

static const char* SMART_TYPE_NAME      = "SMART";


typedef struct smart {
  unsigned char  ts_smart [8];
  unsigned char  ts_faster[8];
} smart;


static size_t SMART_TYPE_SIZE = sizeof (smart);


// Smart tools
unsigned long long smart_get_ts_smart       (faster_data_p data); //  get the smart timestamp
unsigned long long smart_get_ts_faster      (faster_data_p data); //  get the faster timestamp


//  DATA TO STRING (used by faster_disfast)
static inline void smart_attributes_str (faster_data_p data, char* smart_str) {
   long double        hr_clock;
   int                satur     = 0;
   unsigned long long ts_smart  = smart_get_ts_smart  (data);
   unsigned long long ts_faster = smart_get_ts_faster (data);

   strcpy  (smart_str, "");
   sprintf (smart_str, "%s  TS-smart  = %15llu ns", smart_str,ts_smart );
   sprintf (smart_str, "%s  TS-faster = %15llu ns", smart_str,ts_faster);
   sprintf (smart_str, "%s  TS-diff   = %15llu ns", smart_str,ts_smart - ts_faster);
}


#ifdef __cplusplus
}
#endif


#endif  // SMART_H
