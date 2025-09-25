#include <math.h>

#include "fasterac/jdb_hv.h"


  const char* hv_data_Board (hv_data data) {
     switch (data.vmax) {
        case HV_NOTSET:
           return "HV NOT SET";
        case HV_500V:
           if (data.polar == POSITIVE_POLARITY)
              return "500V positive";
           else
              return "500V negative";
        case HV_1KV:
           if (data.polar == POSITIVE_POLARITY)
              return "1kV positive";
           else
              return "1kV negative";
        case HV_2KV:
           if (data.polar == POSITIVE_POLARITY)
              return "2kV positive";
           else
              return "2kV negative";
        case HV_3KV:
           if (data.polar == POSITIVE_POLARITY)
              return "3kV positive";
           else
              return "3kV negative";
        case HV_4KV:
           if (data.polar == POSITIVE_POLARITY)
              return "4kV positive";
           else
              return "4kV negative";
        case HV_6KV:
           if (data.polar == POSITIVE_POLARITY)
              return "6kV positive";
           else
              return "6kV negative";
        default:
           return "UNKNOWN";
     }
  }


  const char* hv_data_State (hv_data data) {
     switch (data.state) {
        case HV_STATE_OFF:
           return "OFF";
        case HV_STATE_ON:
           return "ON";
        case HV_CONFIG_ERROR:
           return "CONFIG ERROR";
        case HV_SWITCH_OFF:
           return "SWITCH OFF";
        case HV_INHIBIT_SIGNAL:
           return "INHIBIT SIGNAL";
        case HV_CURRENT_TRIP:
           return "I TRIP";
        case HV_STATE_ON_RAMP:
           return "ON RAMP";
        case HV_STATE_OFF_RAMP:
           return "OFF RAMP";
        default:
           return "UNKNOWN";
     }
  }


  double hv_data_Current_mA (hv_data data) {
     switch (data.vmax) {
        case HV_500V:
           return data.i_mon * 8.0 / 4096;    //  8.0 mA sur 12bits
        case HV_1KV:
           return data.i_mon * 4.0 / 4096;
        case HV_2KV:
           return data.i_mon * 2.0 / 4096;
        case HV_3KV:
           return data.i_mon * 4.0 / 3.0 / 4096;
        case HV_4KV:
           return data.i_mon * 1.0 / 4096;
        case HV_6KV:
           return data.i_mon * 2.0 / 3.0 / 4096;
     }
  }


  double hv_data_Voltage_V (hv_data data) {
     switch (data.vmax) {
        case HV_500V:
           return data.v_mon * 500.0 / 4096;
        case HV_1KV:
           return data.v_mon * 1000.0 / 4096;
        case HV_2KV:
           return data.v_mon * 2000.0 / 4096;
        case HV_3KV:
           return data.v_mon * 3000.0 / 4096;
        case HV_4KV:
           return data.v_mon * 4000.0 / 4096;
        case HV_6KV:
           return data.v_mon * 6000.0 / 4096;
     }
  }


  double hv_data_Temp_dC (hv_data data) {
     return 0.0625 * data.temp;
  }


