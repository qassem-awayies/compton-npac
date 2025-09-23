#include "fasterac/adc.h"

double adc_delta_t_ns (adc_data adc) {
  return crrc4_spectro_delta_t_ns (adc);
}

