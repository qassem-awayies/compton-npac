
#include <math.h>
#include "fasterac/qdc.h"



double qdc_conv_q_mVns (int q) {
  return q * 0.036468506 * 2.0;        //   CARAS VOLTAGE  LSB : 2*2390 / 2^17 mV
}                                      //            SAMPLING  : 2.0 ns

double qdc_conv_dt_ns  (int tdc) {
  return tdc * 7.8125e-3;              //         HR CLOCK LSB : 2.0 / 256 ns
}

int qdc_conv_dt_raw (double tdc) {
  return round ((tdc / 7.8125e-3));        //         HR CLOCK LSB : 2.0 / 256 ns
}

//  QDC_X1_TYPE_ALIAS     =  41,
//  QDC_X2_TYPE_ALIAS     =  42,
//  QDC_X3_TYPE_ALIAS     =  43,
//  QDC_X4_TYPE_ALIAS     =  44,
//  QDC_TDC_X1_TYPE_ALIAS = 141,
//  QDC_TDC_X2_TYPE_ALIAS = 142,
//  QDC_TDC_X3_TYPE_ALIAS = 143,
//  QDC_TDC_X4_TYPE_ALIAS = 144,
//  QDC_TOF_X1_TYPE_ALIAS = 241,
//  QDC_TOF_X2_TYPE_ALIAS = 242,
//  QDC_TOF_X3_TYPE_ALIAS = 243,
//  QDC_TOF_X4_TYPE_ALIAS = 244,
//  QDC_COUNTER_TYPE_ALIAS=  50

int is_qdc (faster_data_p data) {
   return (qdc_get_nb_q (data) != 0);
}

int qdc_get_nb_q (faster_data_p data) {
   unsigned char alias = faster_data_type_alias (data);
   if ((alias == QDC_X1_TYPE_ALIAS) || (alias == QDC_TDC_X1_TYPE_ALIAS) || (alias == QDC_TOF_X1_TYPE_ALIAS)) return 1;
   if ((alias == QDC_X2_TYPE_ALIAS) || (alias == QDC_TDC_X2_TYPE_ALIAS) || (alias == QDC_TOF_X2_TYPE_ALIAS)) return 2;
   if ((alias == QDC_X3_TYPE_ALIAS) || (alias == QDC_TDC_X3_TYPE_ALIAS) || (alias == QDC_TOF_X3_TYPE_ALIAS)) return 3;
   if ((alias == QDC_X4_TYPE_ALIAS) || (alias == QDC_TDC_X4_TYPE_ALIAS) || (alias == QDC_TOF_X4_TYPE_ALIAS)) return 4;
   return 0;
}

int qdc_get_q_num (faster_data_p data, int num) {
   qdc_x1* q = (qdc_x1*) faster_data_load_p (data);
   return q [num - 1].q1;
}

int qdc_get_sat_num (faster_data_p data, int num) {
   qdc_x1* q = (qdc_x1*) faster_data_load_p (data);
   return q [num - 1].q1_saturated;
}

int qdc_get_tdc (faster_data_p data) {
   qdc_t_x1*     qt1;
   qdc_t_x2*     qt2;
   qdc_t_x3*     qt3;
   qdc_t_x4*     qt4;
   unsigned char alias = faster_data_type_alias (data);
   if (alias == QDC_TDC_X1_TYPE_ALIAS) {
     qt1 = (qdc_t_x1*) faster_data_load_p (data);
     return qt1->tdc;
   }
   if (alias == QDC_TDC_X2_TYPE_ALIAS) {
     qt2 = (qdc_t_x2*) faster_data_load_p (data);
     return qt2->tdc;
   }
   if (alias == QDC_TDC_X3_TYPE_ALIAS) {
     qt3 = (qdc_t_x3*) faster_data_load_p (data);
     return qt3->tdc;
   }
   if (alias == QDC_TDC_X4_TYPE_ALIAS) {
     qt4 = (qdc_t_x4*) faster_data_load_p (data);
     return qt4->tdc;
   }
   return 0;
}

long double qdc_get_tdc_ns (faster_data_p data) {
   return (long double) qdc_conv_dt_ns  (qdc_get_tdc (data));
}

long double qdc_get_hr_clock_ns (faster_data_p data) {
   return (long double) faster_data_clock_ns (data) + qdc_get_tdc_ns (data);
}

int qdc_set_q_num (faster_data_p data, int qdc, int num) {
   qdc_x1* q = (qdc_x1*) faster_data_load_p (data);
   q [num - 1].q1 = qdc;
   return 0;
}

int qdc_set_tdc (faster_data_p data, double tdc) {
   qdc_t_x1*     qt1;
   qdc_t_x2*     qt2;
   qdc_t_x3*     qt3;
   qdc_t_x4*     qt4;
   int tdc_raw         = qdc_conv_dt_raw        (tdc);
   unsigned char alias = faster_data_type_alias (data);
   switch (alias) {
      case QDC_TDC_X1_TYPE_ALIAS :
         qt1 = (qdc_t_x1*) faster_data_load_p (data);
         qt1->tdc = tdc_raw;
         break;
      case QDC_TDC_X2_TYPE_ALIAS :
         qt2 = (qdc_t_x2*) faster_data_load_p (data);
         qt2->tdc = tdc_raw;
         break;
      case QDC_TDC_X3_TYPE_ALIAS :
         qt3 = (qdc_t_x3*) faster_data_load_p (data);
         qt3->tdc = tdc_raw;
         break;
      case QDC_TDC_X4_TYPE_ALIAS :
         qt4 = (qdc_t_x4*) faster_data_load_p (data);
         qt4->tdc = tdc_raw;
         break;
   }
   return 0;
}

//  COUNTER tools
int qdc_counter_set_value   (faster_data_p data, unsigned int calc, unsigned int sent)
{
   qdc_counter* count;
   count = (qdc_counter*) faster_data_load_p (data);
   count->calc = calc;
   count->sent = sent;
   return 0;
}

//  DEPRECATED

double qx1_get_tdc_sec (qdc_t_x1 qdc) {
  return qdc_conv_dt_ns (qdc.tdc) * 1.0e-9;
}

double qx2_get_tdc_sec (qdc_t_x2 qdc) {
  return qdc_conv_dt_ns (qdc.tdc) * 1.0e-9;
}

double qx3_get_tdc_sec (qdc_t_x3 qdc) {
  return qdc_conv_dt_ns (qdc.tdc) * 1.0e-9;
}

double qx4_get_tdc_sec (qdc_t_x4 qdc) {
  return qdc_conv_dt_ns (qdc.tdc) * 1.0e-9;
}

