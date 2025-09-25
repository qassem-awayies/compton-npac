
//
//  QT2T CARAS data definitions
//
//  threshold to threshold charge (auto time window)
//



#ifndef QT2T_H
#define QT2T_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>

#include "fasterac/fasterac.h"


typedef enum {
  QT2T_TYPE_ALIAS         = 46,
  QT2T_COUNTER_TYPE_ALIAS = 52
} qt2t_const;

static const char* QT2T_TYPE_NAME         = "QT2T";
static const char* QT2T_COUNTER_TYPE_NAME = "QT2T_COUNT";


typedef struct qt2t_counter {
  unsigned trig : 32;
  unsigned calc : 32;
  unsigned sent : 32;
} qt2t_counter;


typedef struct qt2t {
  signed   q          : 31;   //  charge                    (thr to thr)
  unsigned saturated  :  1;   //  a sample is saturated     (at least)
  signed   q_baseline : 18;   //  charge before threshold   (baseline)
  unsigned a_max      : 14;   //  max amplitude             (after trigger)
  unsigned w          : 16;   //  width : nb of 2ns samples (thr to thr)
  unsigned t_max      : 16;   //  max position              (sample num after trig)
} qt2t;

static size_t QT2T_TYPE_SIZE         = sizeof (qt2t);
static size_t QT2T_COUNTER_TYPE_SIZE = sizeof (qt2t_counter);
//  DATA TO STRING (used by faster_disfast)

static inline void qt2t_attributes_str (faster_data_p data, char* qt2t_str) {
   qt2t qtt;
   faster_data_load (data, &qtt);
   sprintf (qt2t_str,
            "  Q=%d  width=%dns  max_ampl=%d  max_pos=%dns  baseline=%d", qtt.q,            //  raw charge
                                                                          qtt.w * 2,        //  2ns per sample
                                                                          qtt.a_max,        //  raw amplitude
                                                                          qtt.t_max * 2,    //  2ns per sample
                                                                          qtt.q_baseline);  //  raw charge before trig
   if (qtt.saturated) sprintf (qt2t_str, "%s  saturated", qt2t_str);
}


static inline void qt2t_counter_attributes_str (faster_data_p data, char* count_str) {
   qt2t_counter count;
   faster_data_load (data, &count);
   sprintf (count_str, "  trig=%d  calc=%d  sent=%d", count.trig, count.calc, count.sent);
}


#ifdef __cplusplus
}
#endif


#endif  // QT2T_H
