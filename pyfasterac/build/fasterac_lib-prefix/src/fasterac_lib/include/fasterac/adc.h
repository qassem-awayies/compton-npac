

//
//
//   DEPRECATED :  ADC => Spectro
//
//

#ifndef ADC_H
#define ADC_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "fasterac/spectro.h"

  typedef enum {
    ADC_DATA_TYPE_ALIAS    = CRRC4_SPECTRO_TYPE_ALIAS,
    ADC_COUNTER_TYPE_ALIAS = SPECTRO_COUNTER_TYPE_ALIAS
  } adc_const;

  typedef crrc4_spectro   adc_data;

  typedef spectro_counter adc_counter;

  double adc_delta_t_ns (adc_data adc);

#ifdef __cplusplus
}
#endif


#endif  // ADC_H
