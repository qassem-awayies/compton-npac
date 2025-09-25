
//
//  COMMON FASTER DATA definitions
//



#ifndef FAST_DATA_H
#define FAST_DATA_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "fasterac/fasterac.h"

typedef enum {
  SYNCHRO_TYPE_ALIAS       =   0,
  START_TYPE_ALIAS         =   1,
  STOP_TYPE_ALIAS          =   2,
  MISSING_TYPE_ALIAS       =   3,
  MISSED_TYPE_ALIAS        =   4,
  TREF_TYPE_ALIAS          =  11,
  TREF_TDC_TYPE_ALIAS      =  12,
  UNLOCK_TYPE_ALIAS        =  13,
  OSCILLO_TYPE_ALIAS       =  21,
  OSCILLO_NB_PTS_MAX       = 704,
  OSCILLO_NB_PTS           = OSCILLO_NB_PTS_MAX,  // deprecated
  SAMPLING_TYPE_ALIAS      = OSCILLO_TYPE_ALIAS,  // deprecated
  SAMPLING_NB_PTS          = OSCILLO_NB_PTS       // deprecated
} fast_data_const;

static const char* SYNCHRO_TYPE_NAME    = "SYNCHRO";
static const char* START_TYPE_NAME      = "START";
static const char* STOP_TYPE_NAME       = "STOP";
static const char* MISSING_TYPE_NAME    = "MISSING";
static const char* MISSED_TYPE_NAME     = "MISSED";
static const char* TREF_TYPE_NAME       = "TREF";
static const char* TREF_TDC_TYPE_NAME   = "TREF_TDC";
static const char* UNLOCK_TYPE_NAME     = "UNLOCK";
static const char* OSCILLO_TYPE_NAME    = "OSCILLO";


// OSCILLO
typedef struct oscillo {
  float    x0;                      //   1 * 32
  float  xlsb;                      //   1 * 32
  float  ylsb;                      //   1 * 32
  char   xcap[8];                   //   8 *  8
  char   ycap[8];                   //   8 *  8
  short  samp[OSCILLO_NB_PTS_MAX];  // 704 * 16
} oscillo;

short faster_data_oscillo_nb_pts (faster_data_p data);
float oscillo_x0                 (faster_data_p data);
float oscillo_xlsb               (faster_data_p data);
float oscillo_ylsb               (faster_data_p data);
short oscillo_samples_num        (faster_data_p data);
float oscillo_total_width        (faster_data_p data);
void  oscillo_cap                (faster_data_p data, char* xcap, char* ycap);

static size_t OSCILLO_TYPE_SIZE = sizeof (oscillo);

// SAMPLING
typedef oscillo sampling;  // deprecated


//  TDC
typedef struct tref_tdc {
  signed tdc : 32;
} tref_tdc;


static size_t TREF_TDC_TYPE_SIZE = sizeof (tref_tdc);

//  UNLOCK
typedef struct unlock {
  unsigned multiplicity : 32;
} unlock;


// DATA FIELD CONVERSIONS
double tref_conv_dt_ns  (int tdc      );
double tref_get_tdc_sec (tref_tdc tref);

//  DATA TO STRING (used by faster_disfast)

static inline void tref_tdc_attributes_str (faster_data_p data, char* tref_tdc_str) {
   long double hr_clock;
   tref_tdc    tt_data;
   faster_data_load (data, &tt_data);
   hr_clock = (long double) faster_data_clock_ns (data) + tref_conv_dt_ns (tt_data.tdc);
   sprintf (tref_tdc_str, "  hr_clock=%.3Lfns", hr_clock);
}

static inline void oscillo_attributes_str (faster_data_p data, char* osc_str) {
   oscillo o;
   int     nb_pts;
   faster_data_load (data, &o);
   nb_pts = faster_data_oscillo_nb_pts (data);
   sprintf (osc_str, "  nb_pts=%d  x0=%f  %s  xlsb=%f  %s  ylsb=%f  %s", nb_pts, o.x0, o.xcap, o.xlsb, o.xcap, o.ylsb, o.ycap);
   /*  display 2 points
   sprintf (osc_str, "%s   ", osc_str);
   for (n=0; n<2; n++) {
      sprintf (osc_str, "%s (%f, %f)", osc_str, n * o.xlsb + o.x0, o.samp[n] * o.ylsb);
   }
   printf (osc_str, "%s ...", osc_str);
   */
}

static inline void unlock_attributes_str (faster_data_p data, char* ul_str) {
   unlock ul;
   faster_data_load (data, &ul);
   sprintf (ul_str, "  multiplicity=%d", ul.multiplicity);
}

#ifdef __cplusplus
}
#endif


#endif  // FAST_DATA_H
