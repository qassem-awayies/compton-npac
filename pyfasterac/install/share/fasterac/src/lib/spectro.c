#include "fasterac/spectro.h"
#include <math.h>

#define DELTA_T_LSB_NS 8.0


double crrc4_spectro_delta_t_ns (crrc4_spectro data) {
  return data.delta_t * DELTA_T_LSB_NS;
}

double trapez_spectro_conv_dt_ns (int tdc) {
  return tdc * 0.03125;                   //  sample 2.0 ns + 6bit tdc -> lsb = 2.0 / 2**6
}

int trapez_spectro_conv_dt_raw (double tdc) {
  return round ((tdc / 0.03125));         //  sample 2.0 ns + 6bit tdc -> lsb = 2.0 / 2**6
}

int trapez_spectro_set_tdc (faster_data_p data, double tdc)
{
   trapez_spectro*  trapez;
   int              tdc_raw = trapez_spectro_conv_dt_raw (tdc);

   faster_data_load (data, &trapez);
   trapez->tdc = tdc_raw;
   return 0;
}

int trapez_spectro_set_adc (faster_data_p data, double adc)
{
   trapez_spectro*  trapez;

   faster_data_load (data, &trapez);
   trapez->measure = round (adc);
   return 0;
}

//  COUNTER tools
int spectro_counter_set_value   (faster_data_p data, unsigned int calc, unsigned int sent)
{
   spectro_counter* count;
   count = (spectro_counter*) faster_data_load_p (data);
   count->trig = calc;
   count->calc = calc;
   count->sent = sent;
   return 0;
}
