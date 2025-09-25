#include <stdio.h>
#include <string.h>


#include "fasterac/utils.h"

#include "fasterac/fast_data.h"
#include "fasterac/qdc.h"
#include "fasterac/group.h"
#include "fasterac/spectro.h"
#include "fasterac/rf.h"
#include "fasterac/electrometer.h"
#include "fasterac/scaler.h"
#include "fasterac/qt2t.h"
#include "fasterac/sampler.h"
#include "fasterac/qtdc.h"
#include "fasterac/jdb_hv.h"
#include "fasterac/plas.h"
#include "fasterac/smart.h"

//--------------------------------------------------//

void data_display (faster_data_p data, int n, int tab, int full) {
  relative_data_display (data, n, tab, full, 0, TUNIT_NS);
}

//--------------------------------------------------//

void get_attributes_str (faster_data_p data, unsigned char alias, char* attributes_str) {
  switch (alias) {
    case TREF_TDC_TYPE_ALIAS:
      tref_tdc_attributes_str (data, attributes_str);
      break;
    case OSCILLO_TYPE_ALIAS:
      oscillo_attributes_str (data, attributes_str);
      break;
    case RF_DATA_TYPE_ALIAS:
      rf_data_attributes_str (data, attributes_str);
      break;
    case RF_COUNTER_TYPE_ALIAS:
      rf_counter_attributes_str (data, attributes_str);
      break;
    case QDC_X1_TYPE_ALIAS:
    case QDC_X2_TYPE_ALIAS:
    case QDC_X3_TYPE_ALIAS:
    case QDC_X4_TYPE_ALIAS:
    case QDC_TDC_X1_TYPE_ALIAS:
    case QDC_TDC_X2_TYPE_ALIAS:
    case QDC_TDC_X3_TYPE_ALIAS:
    case QDC_TDC_X4_TYPE_ALIAS:
    case QDC_TOF_X1_TYPE_ALIAS:
    case QDC_TOF_X2_TYPE_ALIAS:
    case QDC_TOF_X3_TYPE_ALIAS:
    case QDC_TOF_X4_TYPE_ALIAS:
      qdc_attributes_str (data, attributes_str);
      break;
    case QDC_COUNTER_TYPE_ALIAS:
      qdc_counter_attributes_str (data, attributes_str);
      break;
    case GROUP_COUNTER_TYPE_ALIAS:
      group_counter_attributes_str (data, attributes_str);
      break;
    case CRRC4_SPECTRO_TYPE_ALIAS:
      crrc4_attributes_str (data, attributes_str);
      break;
    case TRAPEZ_SPECTRO_TYPE_ALIAS:
      trapez_attributes_str (data, attributes_str);
      break;
    case SPECTRO_COUNTER_TYPE_ALIAS:
      spectro_counter_attributes_str (data, attributes_str);
      break;
    case ELECTROMETER_TYPE_ALIAS:
      electrometer_attributes_str (data, attributes_str);
      break;
    case SCALER_MEASUREMENT_TYPE_ALIAS:
      scaler_measurement_attributes_str (data, attributes_str);
      break;
    case SCALER_COUNTER_TYPE_ALIAS:
      scaler_counter_attributes_str (data, attributes_str);
      break;
    case QT2T_TYPE_ALIAS:
      qt2t_attributes_str (data, attributes_str);
      break;
    case QT2T_COUNTER_TYPE_ALIAS:
      qt2t_counter_attributes_str (data, attributes_str);
      break;
    case SAMPLER_DATA_TYPE_ALIAS:
      sampler_attributes_str (data, attributes_str);
      break;
    case SAMPLER_COUNTER_TYPE_ALIAS:
      sampler_counter_attributes_str (data, attributes_str);
      break;
    case QTDC_TYPE_ALIAS:
      qtdc_attributes_str (data, attributes_str);
      break;
    case QTDC_COUNTER_TYPE_ALIAS:
      qtdc_counter_attributes_str (data, attributes_str);
      break;
    case JDB_HV_TYPE_ALIAS:
      hv_attributes_str (data, attributes_str);
      break;
    case PLAS_RAW_DATA_TYPE_ALIAS:
      plas_raw_attributes_str (data, attributes_str);
      break;
    case SMART_TYPE_ALIAS:
      smart_attributes_str (data, attributes_str);
      break;
    case UNLOCK_TYPE_ALIAS:
      unlock_attributes_str (data, attributes_str);
      break;
    default:
      strcpy (attributes_str, "");
  }
}


//--------------------------------------------------//

void group_display (faster_data_p      data,
                    unsigned short     lsize,
                    unsigned long long clock,
                    int                tab,
                    int                full) {
  int                    i;
  faster_data_p          group_data;
  int                    group_n      = 0;
  void*                  group_buffer = faster_data_load_p (data);
  faster_buffer_reader_p group_reader = faster_buffer_reader_open (group_buffer, lsize);
  printf ("\n");
  for (i=0; i<tab; i++) printf ("   ");
  printf ("           -------------------------------------------\n");
  while ((group_data = faster_buffer_reader_next (group_reader)) != NULL) {
    group_n += 1;
    relative_data_display (group_data, group_n, tab+1, full, clock, TUNIT_NS);
  }
  for (i=0; i<tab; i++) printf ("   ");
  printf ("           -------------------------------------------\n");
  faster_buffer_reader_close (group_reader);
}


//--------------------------------------------------//

void relative_data_display (faster_data_p      data,
                            int                n,
                            int                tab,
                            int                full,
                            unsigned long long ref_ns,
                            time_unit          tunit) {
  unsigned char          alias;
  unsigned short         label;
  unsigned short         lsize;
  unsigned long long     clock;
  long long              rel_clock;
  long double            hr_clock;
  char                   clk_str             [256];
  char                   data_attributes_str [2048];
  int                    i;
  //
  alias     = faster_data_type_alias (data);
  label     = faster_data_label      (data);
  lsize     = faster_data_load_size  (data);
  clock     = faster_data_clock_ns   (data);
  rel_clock = clock - ref_ns;
  if (tunit == TUNIT_NS) {
    sprintf (clk_str, "%15.0Lf%s", (long double) rel_clock / tunit_coef [tunit], tunit_str [tunit]);
  } else {
    sprintf (clk_str, "%15.2Lf%s", (long double) rel_clock / tunit_coef [tunit], tunit_str [tunit]);
  }
  for (i=0; i<tab; i++) printf ("   ");
  printf ("%10d  ", n);
  printf ("%15s %5d  %s", type_name (alias), label, clk_str);
  if (alias == GROUP_TYPE_ALIAS) {
     group_display (data, lsize, clock, tab, full);
  } else if (full) {
     get_attributes_str (data, alias, data_attributes_str);
     printf ("%s\n", data_attributes_str);
  }
}


//--------------------------------------------------//

const char* type_name (unsigned char type_alias) {
  switch (type_alias) {
    case SYNCHRO_TYPE_ALIAS:
      return SYNCHRO_TYPE_NAME;
      break;
    case START_TYPE_ALIAS:
      return START_TYPE_NAME;
      break;
    case STOP_TYPE_ALIAS:
      return STOP_TYPE_NAME;
      break;
    case MISSING_TYPE_ALIAS:
      return MISSING_TYPE_NAME;
      break;
    case MISSED_TYPE_ALIAS:
      return MISSED_TYPE_NAME;
      break;
    case UNLOCK_TYPE_ALIAS:
      return UNLOCK_TYPE_NAME;
      break;
    case TREF_TYPE_ALIAS:
      return TREF_TYPE_NAME;
      break;
    case TREF_TDC_TYPE_ALIAS:
      return TREF_TDC_TYPE_NAME;
      break;
    case GROUP_TYPE_ALIAS:
      return GROUP_TYPE_NAME;
      break;
    case GROUP_COUNTER_TYPE_ALIAS:
      return GROUP_COUNTER_TYPE_NAME;
      break;
    case OSCILLO_TYPE_ALIAS:
      return OSCILLO_TYPE_NAME;
      break;
    case RF_DATA_TYPE_ALIAS:
      return RF_DATA_TYPE_NAME;
      break;
    case RF_COUNTER_TYPE_ALIAS:
      return RF_COUNTER_TYPE_NAME;
      break;
    case QDC_X1_TYPE_ALIAS:
      return QDC_X1_TYPE_NAME;
      break;
    case QDC_X2_TYPE_ALIAS:
      return QDC_X2_TYPE_NAME;
      break;
    case QDC_X3_TYPE_ALIAS:
      return QDC_X3_TYPE_NAME;
      break;
    case QDC_X4_TYPE_ALIAS:
      return QDC_X4_TYPE_NAME;
      break;
    case QDC_TDC_X1_TYPE_ALIAS:
      return QDC_TDC_X1_TYPE_NAME;
      break;
    case QDC_TDC_X2_TYPE_ALIAS:
      return QDC_TDC_X2_TYPE_NAME;
      break;
    case QDC_TDC_X3_TYPE_ALIAS:
      return QDC_TDC_X3_TYPE_NAME;
      break;
    case QDC_TDC_X4_TYPE_ALIAS:
      return QDC_TDC_X4_TYPE_NAME;
      break;
    case QDC_TOF_X1_TYPE_ALIAS:
      return QDC_TOF_X1_TYPE_NAME;
      break;
    case QDC_TOF_X2_TYPE_ALIAS:
      return QDC_TOF_X2_TYPE_NAME;
      break;
    case QDC_TOF_X3_TYPE_ALIAS:
      return QDC_TOF_X3_TYPE_NAME;
      break;
    case QDC_TOF_X4_TYPE_ALIAS:
      return QDC_TOF_X4_TYPE_NAME;
      break;
    case QDC_COUNTER_TYPE_ALIAS:
      return QDC_COUNTER_TYPE_NAME;
      break;
    case CRRC4_SPECTRO_TYPE_ALIAS:
      return CRRC4_SPECTRO_TYPE_NAME;
      break;
    case TRAPEZ_SPECTRO_TYPE_ALIAS:
      return TRAPEZ_SPECTRO_TYPE_NAME;
      break;
    case SPECTRO_COUNTER_TYPE_ALIAS:
      return SPECTRO_COUNTER_TYPE_NAME;
      break;
    case ELECTROMETER_TYPE_ALIAS:
      return ELECTROMETER_TYPE_NAME;
      break;
    case SCALER_MEASUREMENT_TYPE_ALIAS:
      return SCALER_MEASUREMENT_TYPE_NAME;
      break;
    case SCALER_COUNTER_TYPE_ALIAS:
      return SCALER_COUNTER_TYPE_NAME;
      break;
    case QT2T_TYPE_ALIAS:
      return QT2T_TYPE_NAME;
      break;
    case QT2T_COUNTER_TYPE_ALIAS:
      return QT2T_COUNTER_TYPE_NAME;
      break;
    case SAMPLER_DATA_TYPE_ALIAS:
      return SAMPLER_TYPE_NAME;
      break;
    case SAMPLER_COUNTER_TYPE_ALIAS:
      return SAMPLER_COUNTER_TYPE_NAME;
      break;
    case QTDC_TYPE_ALIAS:
      return QTDC_TYPE_NAME;
      break;
    case QTDC_COUNTER_TYPE_ALIAS:
      return QTDC_COUNTER_TYPE_NAME;
      break;
    case JDB_HV_TYPE_ALIAS:
      return JDB_HV_TYPE_NAME;
      break;
    case PLAS_RAW_DATA_TYPE_ALIAS:
      return PLAS_RAW_TYPE_NAME;
    case SMART_TYPE_ALIAS:
      return SMART_TYPE_NAME;
      break;
    default:
      return "UNKNOWN DATA";
  }
}


//--------------------------------------------------//

const unsigned short type_size (unsigned char type_alias) {
  switch (type_alias) {
     case TREF_TDC_TYPE_ALIAS:
        return TREF_TDC_TYPE_SIZE;
        break;
     case OSCILLO_TYPE_ALIAS:
        return OSCILLO_TYPE_SIZE;
        break;
     case RF_DATA_TYPE_ALIAS:
        return RF_DATA_TYPE_SIZE;
        break;
     case RF_COUNTER_TYPE_ALIAS:
        return RF_COUNTER_TYPE_SIZE;
        break;
     case QDC_X1_TYPE_ALIAS:
        return QDC_X1_TYPE_SIZE;
        break;
     case QDC_X2_TYPE_ALIAS:
        return QDC_X2_TYPE_SIZE;
        break;
     case QDC_X3_TYPE_ALIAS:
        return QDC_X3_TYPE_SIZE;
        break;
     case QDC_X4_TYPE_ALIAS:
        return QDC_X4_TYPE_SIZE;
        break;
     case QDC_TDC_X1_TYPE_ALIAS:
        return QDC_TDC_X1_TYPE_SIZE;
        break;
     case QDC_TDC_X2_TYPE_ALIAS:
        return QDC_TDC_X2_TYPE_SIZE;
        break;
     case QDC_TDC_X3_TYPE_ALIAS:
        return QDC_TDC_X3_TYPE_SIZE;
        break;
     case QDC_TDC_X4_TYPE_ALIAS:
        return QDC_TDC_X4_TYPE_SIZE;
        break;
     case QDC_COUNTER_TYPE_ALIAS:
        return QDC_COUNTER_TYPE_SIZE;
        break;
     case GROUP_COUNTER_TYPE_ALIAS:
        return GROUP_COUNTER_TYPE_SIZE;
        break;
     case CRRC4_SPECTRO_TYPE_ALIAS:
       return CRRC4_SPECTRO_TYPE_SIZE;
       break;
     case TRAPEZ_SPECTRO_TYPE_ALIAS:
       return TRAPEZ_SPECTRO_TYPE_SIZE;
       break;
     case SPECTRO_COUNTER_TYPE_ALIAS:
       return SPECTRO_COUNTER_TYPE_SIZE;
       break;
     case SCALER_MEASUREMENT_TYPE_ALIAS:
       return SCALER_MEASUREMENT_TYPE_SIZE;
       break;
     case SCALER_COUNTER_TYPE_ALIAS:
       return SCALER_COUNTER_TYPE_SIZE;
       break;
     case QT2T_TYPE_ALIAS:
       return QT2T_TYPE_SIZE;
       break;
     case QT2T_COUNTER_TYPE_ALIAS:
       return QT2T_COUNTER_TYPE_SIZE;
       break;
     case SAMPLER_DATA_TYPE_ALIAS:
       return SAMPLER_TYPE_SIZE;
       break;
     case SAMPLER_COUNTER_TYPE_ALIAS:
       return SAMPLER_COUNTER_TYPE_SIZE;
       break;
     case PLAS_RAW_DATA_TYPE_ALIAS:
       return PLAS_RAW_DATA_TYPE_SIZE;
     case SMART_TYPE_ALIAS:
       return SMART_TYPE_SIZE;
       break;
     default:
       return -1;
  }
}
//--------------------------------------------------//


long double conv_tdc_to_ns (int tdc_reg) {
  return tdc_reg * 7.8125e-3;              //   HR CLOCK LSB : 2.0 / 2**8 ns
}

long double tdc_ns (const faster_data_p data) {
  switch (faster_data_type_alias (data)) {
    case TREF_TDC_TYPE_ALIAS:
      return conv_tdc_to_ns (((tref_tdc*) faster_data_load_p (data))->tdc);
    case QDC_TDC_X1_TYPE_ALIAS:
      return conv_tdc_to_ns (((qdc_t_x1*) faster_data_load_p (data))->tdc);
    case QDC_TDC_X2_TYPE_ALIAS:
      return conv_tdc_to_ns (((qdc_t_x2*) faster_data_load_p (data))->tdc);
    case QDC_TDC_X3_TYPE_ALIAS:
      return conv_tdc_to_ns (((qdc_t_x3*) faster_data_load_p (data))->tdc);
    case QDC_TDC_X4_TYPE_ALIAS:
      return conv_tdc_to_ns (((qdc_t_x4*) faster_data_load_p (data))->tdc);
    case QTDC_TYPE_ALIAS:
      return qtdc_tdc_ns (*((qtdc*)faster_data_load_p (data)));
    case RF_DATA_TYPE_ALIAS:
      return conv_tdc_to_ns (((rf_data*)  faster_data_load_p (data))->trig_dt);
    case TRAPEZ_SPECTRO_TYPE_ALIAS:
      return trapez_spectro_conv_dt_ns (((trapez_spectro*)  faster_data_load_p (data))->tdc);
    default:
      return 0.0;
  }
}


long double faster_data_hr_clock_ns (const faster_data_p data) {
  long double t  = faster_data_clock_ns (data);
  long double dt = tdc_ns               (data);
  return t + dt;
}


long double faster_data_hr_clock_sec (const faster_data_p data) {
  return faster_data_hr_clock_ns (data) * 1e-9L;
}


long double faster_data_rf_time_ns (const faster_data_p data, const faster_data_p rf_ref) {
  rf_data     *rf;
  long double meas_data_time;
  long double meas_rf_time;
  long double rf_period;
  long double last_rf_time;

  if (faster_data_type_alias (rf_ref) != RF_DATA_TYPE_ALIAS) return -1;

  rf             = faster_data_load_p (rf_ref);
  rf_period      = rf_period_ns (*rf);
  meas_data_time = faster_data_hr_clock_ns (data);
  meas_rf_time   = (long double)faster_data_clock_ns (rf) + rf_pll_dt_ns (*rf);
  last_rf_time   = meas_rf_time + floorl ((meas_data_time - meas_rf_time) / rf_period) * rf_period;
  return meas_data_time - last_rf_time;
}

//--------------------------------------------------//
