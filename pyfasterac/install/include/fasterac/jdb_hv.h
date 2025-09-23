
//
//  JDB HV data definitions
//



#ifndef JDB_HV_H
#define JDB_HV_H 1

#ifdef __cplusplus
extern "C" {
#endif

  #include <string.h>
  #include <stdio.h>

  #include "fasterac/fasterac.h"


  typedef enum {
    JDB_HV_TYPE_ALIAS = 84
  } jdb_hv_const;

  static const char* JDB_HV_TYPE_NAME = "HV";



  typedef enum Vmax_t {
    HV_NOTSET = 0x00,
    HV_500V   = 0x11,
    HV_1KV    = 0x12,
    HV_2KV    = 0x13,
    HV_3KV    = 0x14,
    HV_4KV    = 0x15,
    HV_6KV    = 0x16
  } Vmax_t;


  typedef enum Polar_t {
     NEGATIVE_POLARITY = 0,
     POSITIVE_POLARITY = 1
  } Polar_t;


  typedef enum State_t {
    HV_STATE_OFF      = 0x0,
    HV_STATE_ON       = 0x1,
    HV_CONFIG_ERROR   = 0x2,
    HV_SWITCH_OFF     = 0x3,
    HV_INHIBIT_SIGNAL = 0x4,
    HV_CURRENT_TRIP   = 0x5,
    HV_STATE_ON_RAMP  = 0x6,
    HV_STATE_OFF_RAMP = 0x7,
    HV_STATE_UNKNOWN  = 0xF
  } State_t;


  typedef struct hv_data {
    unsigned state :  4;    //  State_t
    unsigned i_mon : 12;
    unsigned v_mon : 12;
    unsigned p0    :  4;
    unsigned polar :  1;    //  Polar_t
    unsigned p1    :  7;
    unsigned vmax  :  8;    //  Vmax_t
    unsigned p2    :  4;
    unsigned temp  : 12;
  } hv_data;


  // Data handling
  const char*  hv_data_Board      (hv_data data);
  const char*  hv_data_State      (hv_data data);
  double       hv_data_Current_mA (hv_data data);
  double       hv_data_Voltage_V  (hv_data data);
  double       hv_data_Temp_dC    (hv_data data);


  //  DATA TO STRING (used by faster_disfast)

  static inline void hv_attributes_str (faster_data_p data, char* hv_str) {
    hv_data hv;
    faster_data_load (data, &hv);
    sprintf (hv_str,
             "  board=%s  state=%s  hv=%.2fV  i=%.2fmA  t=%.1f°C", hv_data_Board      (hv),
                                                                   hv_data_State      (hv),
                                                                   hv_data_Voltage_V  (hv),
                                                                   hv_data_Current_mA (hv),
                                                                   hv_data_Temp_dC    (hv));
  }


#ifdef __cplusplus
}
#endif


#endif  // JDB_HV_H
