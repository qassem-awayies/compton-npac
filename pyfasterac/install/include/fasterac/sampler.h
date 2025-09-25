
//
//  Sampler CARAS data definitions
//



#ifndef SAMPLER_H
#define SAMPLER_H 1

#ifdef __cplusplus
extern "C" {
#endif


#include <string.h>
#include <stdio.h>

#include "fasterac/fasterac.h"


//  CONST

typedef enum {
   SAMPLER_DATA_TYPE_ALIAS    =  22,
   SAMPLER_COUNTER_TYPE_ALIAS =  23,
   //SAMPLER_NB_PTS_MAX         = 714
   SAMPLER_NB_PTS_MAX         = 718
} sampler_const;

typedef enum {
   SAMPLER_RANGE_150MV   =  0,
   SAMPLER_RANGE_300MV   =  1,
   SAMPLER_RANGE_600MV   =  2,
   SAMPLER_RANGE_1200MV  =  3,
   SAMPLER_RANGE_2400MV  =  4
} sampler_range_t;

static const char* SAMPLER_RANGE_TEXT[]={
   "RANGE_150MV",
   "RANGE_300MV" ,
   "RANGE_600MV" ,
   "RANGE_1200MV",
   "RANGE_2400MV"
};

static const char*  SAMPLER_TYPE_NAME         = "SAMPLER";
static const char*  SAMPLER_COUNTER_TYPE_NAME = "SAMPLER_COUNT";
static const size_t SAMPLER_TYPE_SIZE         = SAMPLER_NB_PTS_MAX;


//  DATA DEFINITIONS

typedef short* sampler_data;

typedef struct sampler_header_t {

  unsigned     output_range :3 ;
  unsigned     samples_num  :11;
  unsigned     before_th    :10;
  unsigned     zero         :8;
} sampler_header_t;

typedef struct sampler_frame_t {
  sampler_header_t header;
  short            data[SAMPLER_NB_PTS_MAX];
} sampler;

typedef sampler_header_t* sampler_header_p;

typedef struct sampler_counter {
  unsigned trig : 32;
  unsigned calc : 32;
  unsigned sent : 32;
} sampler_counter;

static const size_t SAMPLER_COUNTER_TYPE_SIZE = sizeof (sampler_counter);;

// SAMPLER tools
int         sampler_samples_num      (faster_data_p data);
int         sampler_total_width      (faster_data_p data);
int         sampler_before_th_ns     (faster_data_p data);
int         sampler_before_th        (faster_data_p data);
int         sampler_output_range     (faster_data_p data);
const char* sampler_output_range_str (faster_data_p data);

double      sampler_conv_raw_to_mV   (double raw_data, sampler_range_t range);

//  DATA TO STRING (used by faster_disfast)

static inline void sampler_attributes_str (faster_data_p data, char* samp_str) {
  sampler          s;
  short            i;
  int              load_size = faster_data_load_size (data);
  int              nb_pts;


  sprintf (samp_str, "  before_trig=%dns width=%dns nb_of_pts=%d range=%s --   ", sampler_before_th_ns     (data)
                                                                                , sampler_total_width      (data)
                                                                                , sampler_samples_num      (data)
                                                                                , sampler_output_range_str (data));
  faster_data_load (data, &s);
  nb_pts = sampler_samples_num (data);

  if (nb_pts <= 10) {
    for (i=0; i<nb_pts; i++) sprintf (samp_str, "%s%d  ", samp_str, s.data[i]);
  } else {
    for (i=0; i<10; i++)     sprintf (samp_str, "%s%d  ", samp_str, s.data[i]);
                             sprintf (samp_str, "%s ...", samp_str);
  }
}


static inline void sampler_counter_attributes_str (faster_data_p data, char* count_str) {
   sampler_counter count;
   faster_data_load (data, &count);
   sprintf (count_str, "  trig=%d  calc=%d  sent=%d", count.trig, count.calc, count.sent);
}


#ifdef __cplusplus
}
#endif


#endif  // SAMPLER_H
