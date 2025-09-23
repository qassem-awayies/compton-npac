

#include "fasterac/scaler.h"

//  ylsb = 0.004558563 mV    (2*2390 / 2^20)
#define TO_MV 0.004558563

//  xlsb = 2ns
#define TO_NS 2.0


double scaler_max_ampl_mV (scaler_measurement scaler) {
  return scaler.max_ampl * TO_MV;
}


double scaler_max_pos_ns (scaler_measurement scaler) {
  return scaler.max_pos * TO_NS;
}

double scaler_qtt_mVns    (scaler_measurement scaler) {
  return scaler.qtt * TO_MV * TO_NS;
}


double scaler_fw_thres_ns (scaler_measurement scaler) {
  return scaler.fw_thres * TO_NS;
}





