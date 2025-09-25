#include <string.h>
#include "fasterac/fast_data.h"

short faster_data_oscillo_nb_pts (faster_data_p data) {
   short lsize = 0;
   short nbpts = 0;
   if (faster_data_type_alias (data) == OSCILLO_TYPE_ALIAS) {
      lsize = faster_data_load_size (data);
      nbpts = (lsize - 28) / 2;
   }
   return nbpts;
}

float oscillo_x0 (faster_data_p data)
{
   oscillo* o = (oscillo*) faster_data_load_p (data);
   return o->x0;
}

float oscillo_xlsb (faster_data_p data)
{
   oscillo* o = (oscillo*) faster_data_load_p (data);
   return o->xlsb;
}

float oscillo_ylsb (faster_data_p data)
{
   oscillo* o = (oscillo*) faster_data_load_p (data);
   return o->ylsb;
}
short oscillo_samples_num (faster_data_p data)
{
   return faster_data_oscillo_nb_pts (data);
}

float oscillo_total_width (faster_data_p data)
{
   oscillo* o      = (oscillo*) faster_data_load_p (data);
   short    nb_pts = faster_data_oscillo_nb_pts (data);
   return nb_pts * o->xlsb;
}

void oscillo_cap (faster_data_p data, char* xcap, char* ycap)
{

   oscillo* o = (oscillo*) faster_data_load_p (data);
   strcpy (xcap, o->xcap);
   strcpy (ycap, o->ycap);
}

double tref_conv_dt_ns  (int tdc) {
  return tdc * 7.8125e-3;              //        HR CLOCK LSB : 2.0 / 256 ns
}



double tref_get_tdc_sec (tref_tdc tref) {
  return tref_conv_dt_ns (tref.tdc) * 1.0e-9;
}


