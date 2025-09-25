#include <math.h>
#include "fasterac/sampler.h"


double  sampler_conv_raw_to_mV   (double raw_data, sampler_range_t range)
{
   switch (range)
   {
      case SAMPLER_RANGE_150MV :
         return raw_data * 0.004558563; // 2390/2^19
         break;
      case SAMPLER_RANGE_300MV :
         return raw_data * 0.009117126; // 2390/2^18
         break;
      case SAMPLER_RANGE_600MV :
         return raw_data * 0.018234253; // 2390/2^17
         break;
      case SAMPLER_RANGE_1200MV:
         return raw_data * 0.036468506; // 2390/2^16
         break;
      case SAMPLER_RANGE_2400MV:
         return raw_data * 0.072631836; // 2390/2^15
         break;
      default :
         printf ("==> Wrong range value\n");
         break;
   }
}

//double qdc_conv_q_mVns (int q) {
//  return q * 0.036468506 * 2.0;        //   CARAS VOLTAGE  LSB : 2*2390 / 2^17 mV
//}                                      //            SAMPLING  : 2.0 ns

//double qdc_conv_dt_ns  (int tdc) {
//  return tdc * 7.8125e-3;              //         HR CLOCK LSB : 2.0 / 256 ns
//}

//int qdc_conv_dt_raw (double tdc) {
//  return round ((tdc / 7.8125e-3));        //         HR CLOCK LSB : 2.0 / 256 ns
//}

// SAMPLER tools

int sampler_samples_num  (faster_data_p data)
{
   sampler* s = (sampler*) faster_data_load_p (data);
   return s->header.samples_num;
}

int sampler_total_width  (faster_data_p data)
{
   return sampler_samples_num (data) * 2 ;
}

int sampler_before_th    (faster_data_p data)
{
   sampler* s = (sampler*) faster_data_load_p (data);
   return 40 - s->header.before_th;
}

int sampler_before_th_ns (faster_data_p data)
{
    return sampler_before_th (data) * 2 ;
}

int sampler_output_range   (faster_data_p data)
{
   sampler* s = (sampler*) faster_data_load_p (data);
   return s->header.output_range;
}

const char* sampler_output_range_str   (faster_data_p data)
{
   return SAMPLER_RANGE_TEXT [sampler_output_range (data)];
}
