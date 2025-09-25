

#include "fasterac/rf.h"

//#define RF_PERIOD_TICK_NS 1.9073486328125e-6          //      1.0 / pow (2.0, 19) ns
#define RF_PERIOD_TICK_NS 1.0 / pow (2.0, 19)           //      1.0 / pow (2.0, 19) ns
#define RF_TRIG_TICK_NS   7.8125e-3                     //      2.0 / 256 ns


double rf_period_ns  (rf_data rf) {
  return rf.period * RF_PERIOD_TICK_NS;
}


double rf_trig_dt_ns (rf_data rf) {
  return rf.trig_dt * RF_TRIG_TICK_NS;
}


double rf_pll_dt_ns  (rf_data rf) {
  return rf.pll_dt * RF_TRIG_TICK_NS;
}





