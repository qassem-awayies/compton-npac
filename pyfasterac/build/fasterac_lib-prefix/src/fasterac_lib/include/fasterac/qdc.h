
//
//  QDC data definitions
//



#ifndef QDC_H
#define QDC_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>

#include "fasterac/fasterac.h"


typedef enum {
  QDC_X1_TYPE_ALIAS     =  41,
  QDC_X2_TYPE_ALIAS     =  42,
  QDC_X3_TYPE_ALIAS     =  43,
  QDC_X4_TYPE_ALIAS     =  44,
  QDC_TDC_X1_TYPE_ALIAS = 141,
  QDC_TDC_X2_TYPE_ALIAS = 142,
  QDC_TDC_X3_TYPE_ALIAS = 143,
  QDC_TDC_X4_TYPE_ALIAS = 144,
  QDC_TOF_X1_TYPE_ALIAS = 241,
  QDC_TOF_X2_TYPE_ALIAS = 242,
  QDC_TOF_X3_TYPE_ALIAS = 243,
  QDC_TOF_X4_TYPE_ALIAS = 244,
  QDC_COUNTER_TYPE_ALIAS=  50
} qdc_const;

static const char* QDC_X1_TYPE_NAME      = "QDC_X1";
static const char* QDC_X2_TYPE_NAME      = "QDC_X2";
static const char* QDC_X3_TYPE_NAME      = "QDC_X3";
static const char* QDC_X4_TYPE_NAME      = "QDC_X4";
static const char* QDC_TDC_X1_TYPE_NAME  = "QDC_TDC_X1";
static const char* QDC_TDC_X2_TYPE_NAME  = "QDC_TDC_X2";
static const char* QDC_TDC_X3_TYPE_NAME  = "QDC_TDC_X3";
static const char* QDC_TDC_X4_TYPE_NAME  = "QDC_TDC_X4";
static const char* QDC_TOF_X1_TYPE_NAME  = "QDC_TOF_X1";
static const char* QDC_TOF_X2_TYPE_NAME  = "QDC_TOF_X2";
static const char* QDC_TOF_X3_TYPE_NAME  = "QDC_TOF_X3";
static const char* QDC_TOF_X4_TYPE_NAME  = "QDC_TOF_X4";
static const char* QDC_COUNTER_TYPE_NAME = "QDC_COUNTER";


typedef struct qdc_x1 {
  signed   q1           : 31;
  unsigned q1_saturated :  1;
} qdc_x1;


typedef struct qdc_x2 {
  signed   q1           : 31;
  unsigned q1_saturated :  1;
  signed   q2           : 31;
  unsigned q2_saturated :  1;
} qdc_x2;


typedef struct qdc_x3 {
  signed   q1           : 31;
  unsigned q1_saturated :  1;
  signed   q2           : 31;
  unsigned q2_saturated :  1;
  signed   q3           : 31;
  unsigned q3_saturated :  1;
} qdc_x3;


typedef struct qdc_x4 {
  signed   q1           : 31;
  unsigned q1_saturated :  1;
  signed   q2           : 31;
  unsigned q2_saturated :  1;
  signed   q3           : 31;
  unsigned q3_saturated :  1;
  signed   q4           : 31;
  unsigned q4_saturated :  1;
} qdc_x4;


typedef struct qdc_t_x1 {
  signed   q1           : 31;
  unsigned q1_saturated :  1;
  signed   tdc          : 32;
} qdc_t_x1;


typedef struct qdc_t_x2 {
  signed   q1           : 31;
  unsigned q1_saturated :  1;
  signed   q2           : 31;
  unsigned q2_saturated :  1;
  signed   tdc          : 32;
} qdc_t_x2;


typedef struct qdc_t_x3 {
  signed   q1           : 31;
  unsigned q1_saturated :  1;
  signed   q2           : 31;
  unsigned q2_saturated :  1;
  signed   q3           : 31;
  unsigned q3_saturated :  1;
  signed   tdc          : 32;
} qdc_t_x3;


typedef struct qdc_t_x4 {
  signed   q1           : 31;
  unsigned q1_saturated :  1;
  signed   q2           : 31;
  unsigned q2_saturated :  1;
  signed   q3           : 31;
  unsigned q3_saturated :  1;
  signed   q4           : 31;
  unsigned q4_saturated :  1;
  signed   tdc          : 32;
} qdc_t_x4;


typedef struct qdc_counter {
  unsigned calc : 32;
  unsigned sent : 32;
} qdc_counter;

static size_t QDC_X1_TYPE_SIZE      = sizeof (qdc_x1);
static size_t QDC_X2_TYPE_SIZE      = sizeof (qdc_x2);
static size_t QDC_X3_TYPE_SIZE      = sizeof (qdc_x3);
static size_t QDC_X4_TYPE_SIZE      = sizeof (qdc_x4);

static size_t QDC_TDC_X1_TYPE_SIZE  = sizeof (qdc_t_x1);
static size_t QDC_TDC_X2_TYPE_SIZE  = sizeof (qdc_t_x2);
static size_t QDC_TDC_X3_TYPE_SIZE  = sizeof (qdc_t_x3);
static size_t QDC_TDC_X4_TYPE_SIZE  = sizeof (qdc_t_x4);

static size_t QDC_COUNTER_TYPE_SIZE = sizeof (qdc_counter);

// DATA FIELD CONVERSIONS
double qdc_conv_q_mVns (int q);
double qdc_conv_dt_ns  (int tdc);

// QDC tools
int          is_qdc              (faster_data_p data);                   //  0 => is not a qdc
int          qdc_get_nb_q        (faster_data_p data);                   //  0, 1, 2, 3, 4  (0 => is not a qdc)
int          qdc_get_q_num       (faster_data_p data, int num);          //  charge number num
int          qdc_get_sat_num     (faster_data_p data, int num);          //  1 : measurement num is saturated
int          qdc_get_tdc         (faster_data_p data);                   //  raw tdc field
long double  qdc_get_tdc_ns      (faster_data_p data);                   //  tdc field in ns
long double  qdc_get_hr_clock_ns (faster_data_p data);                   //  qdc hr clock (tdc included)
int          qdc_set_tdc         (faster_data_p data, double tdc);       //  set tdc field
int          qdc_set_q_num       (faster_data_p data, int qdc, int num); //  set charge number num

//  COUNTER tools
int          qdc_counter_set_value   (faster_data_p data, unsigned int calc, unsigned int sent); //  set calc and sent counter

//  DATA TO STRING (used by faster_disfast)
static inline void qdc_attributes_str (faster_data_p data, char* qdc_str) {
   int         q [5];
   int         s [5];
   long double hr_clock;
   int         satur = 0;
   int         nb_q  = qdc_get_nb_q (data);
   int         tdc   = qdc_get_tdc  (data);
   int         i;
   for (i=1; i<=nb_q; i++) {
      q [i] = qdc_get_q_num   (data, i);
      s [i] = qdc_get_sat_num (data, i);
      if (s [i]) satur = 1;
   }
   strcpy (qdc_str, "");
   for (i=1; i<=nb_q; i++) sprintf (qdc_str, "%s  q%d=%d",          qdc_str, i, q [i]);
   if (tdc)                sprintf (qdc_str, "%s  hr_clock=%3Lfns", qdc_str, qdc_get_hr_clock_ns (data));
   if (satur) {
                           sprintf (qdc_str, "%s  saturated :",     qdc_str);
      for (i=1; i<=nb_q; i++) if (s [i])
                           sprintf (qdc_str, "%s q%d",              qdc_str, i);
   }
}

static inline void qdc_counter_attributes_str (faster_data_p data, char* count_str) {
   qdc_counter count;
   faster_data_load (data, &count);
   sprintf (count_str, "  calc=%d  sent=%d", count.calc, count.sent);
}

// DEPRECATED

double qx1_get_tdc_sec (qdc_t_x1 qdc);
double qx2_get_tdc_sec (qdc_t_x2 qdc);
double qx3_get_tdc_sec (qdc_t_x3 qdc);
double qx4_get_tdc_sec (qdc_t_x4 qdc);

#ifdef __cplusplus
}
#endif


#endif  // QDC_H
