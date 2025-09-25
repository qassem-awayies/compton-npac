
#include <math.h>
#include <string.h>
#include <limits.h>

#include "fasterac/sampler.h"
#include "fasterac/sampling.h"

//  ylsb = 0.004558563 mV    (2*2390 / 2^20)
#define TO_MV 0.004558563
#define TO_NS 2.0
#define NOVAL -1000000.0

//----------  local tools  ---------------------------------//


   double sampling_xpoint_ns_order1 (sampling_t const* s,          //  LINEAR INPERPOLATION
                                     double            xval_mV,
                                     int               rising_edge,
                                     double            from_ns,
                                     double            to_ns);

   double sampling_xpoint_ns_order2 (sampling_t const* s,          //  POLY 2
                                     double            xval_mV,
                                     int               rising_edge,
                                     double            from_ns,
                                     double            to_ns);

   double i2ns (double first_ns, unsigned short i) {
      return first_ns + i * TO_NS;
   }

   unsigned short ns2i (double first_ns, double ns) {
      return round ((ns - first_ns) / TO_NS);
   }

   double raw2mV (sampler_range_t range,
                  short           rawdata) {
      return rawdata * pow (2, range) * TO_MV;
   }

   double raw_value (sampling_t const* s,
                     unsigned int      i) {
      //return raw2mV (s->range_mV, s->raw.data [i]);
      return s->raw.data [i];
   }

   double cfd_value (sampling_t const* s,
                     unsigned int      i) {
      //return raw2mV (s->range_mV, s->cfd.data [i]);
      return s->cfd.data [i];
   }

   void cfd_data (sampling_t* s) {

      short  i;

      for (i=0; i <  s->cfd.nb_pts; i++) {
        s->cfd.data [i] = 0;
      }

      //for (i=s->cfd.delay_ns; i < (s->cfd.nb_pts) - s->cfd.delay_ns; i++) {
      for (i=s->cfd.delay_ns; i < (s->cfd.nb_pts) - s->cfd.delay_ns; i++) {
        s->cfd.data [i] = s->raw.data[i]/s->cfd.fraction - s->raw.data[i-s->cfd.delay_ns];
      }
      //fprintf (stdout, "CDF VALUES\n");
      //for (i=0; i<s->cfd.nb_pts; i++) {
      //  fprintf (stdout, "%d->%d ,", i, s->cfd.data[i]);
      //}
      //fprintf (stdout, "\n");
   }

//----------------------------------------------------------//

//   int sampling_init (sampling_t*      s,        //  return 0 when ok
//                      sampler_meta_t*  meta,     //  sampler meta data (label, time shift & voltage range)
//                      faster_data_p    sampler)  //  sampler as faster data
//   {
//      unsigned short  label        = meta->label;
//      unsigned short  type_alias   = meta->type_alias;
//      double          before_th_ns = TO_NS * round ((-1 * meta->shift_ns) / TO_NS);
//      sampler_range_t range_mV     = meta->range_mV;
//      if ((before_th_ns < 0.0) || (before_th_ns >= SAMPLER_NB_PTS_MAX * TO_NS))
//      {
//         return 1;  //  error
//      } else {
//         s->range_mV        = range_mV;
//         s->label           = label;
//         s->type_alias      = type_alias;
//         s->ts_ns           = faster_data_clock_ns  (sampler);
//         s->raw.nb_pts      = faster_data_load_size (sampler) / 2;
//         s->cfd.nb_pts      = faster_data_load_size (sampler) / 2; //TO CHANGE
//         s->raw.first_ns    = -1.0 * before_th_ns;
//         s->raw.last_ns     = i2ns (s->raw.first_ns, s->raw.nb_pts - 1);
//         s->cfd.delay_ns    = meta->cfd_delay_ns;
//         s->cfd.fraction    = meta->cfd_fraction;
//         s->cfd.ts_shift_ns = 0;
//
//         memcpy (s->raw.data, faster_data_load_p (sampler), 2 * s->raw.nb_pts);
//         s->raw.data[0] = s->raw.data[1] = 0; //BUG in sampler mmns
//         if (s->cfd.fraction != 0) cfd_data (s);
//         //switch (s->type_alias) {
//         //    case QDC_TDC_X1_TYPE_ALIAS :
//         //    case QDC_TDC_X2_TYPE_ALIAS :
//         //    case QDC_TDC_X3_TYPE_ALIAS :
//         //    case QDC_TDC_X4_TYPE_ALIAS :
//         //       cfd_data (s);
//         //       break;
//         //}
//         printf ("Sampling Info : label=%d, type_alias=%d, TS=%lf, Nb_pts=%d, First_ns=%lf, Last_ns=%lf\n" , s->label       , s->type_alias,
//                                                                                                             s->ts_ns       , s->raw.nb_pts,
//                                                                                                             s->raw.first_ns, s->raw.last_ns);
//         //  max, min, qtot
//         {
//            double val;
//            double qtot = 0.0;
//            double max  = INT_MIN;
//            double min  = INT_MAX;
//            int    maxi = 0;
//            int    mini = 0;
//            int    i;
//            for (i = 0; i < s->raw.nb_pts; i++) {
//               val = raw_value (s, i);
//               qtot += val * TO_NS;
//               if (val > max) {
//                  maxi = i;
//                  max  = val;
//               }
//               if (val < min) {
//                  mini = i;
//                  min  = val;
//               }
//            }
//            s->raw.max_mV    = max;
//            s->raw.min_mV    = min;
//            s->raw.max_ns    = i2ns (s->raw.first_ns, maxi);
//            s->raw.min_ns    = i2ns (s->raw.first_ns, mini);
//            s->raw.qtot_mVns = qtot;
//            max  = INT_MIN;
//            min  = INT_MAX;
//            for (i = 0; i < s->cfd.nb_pts; i++) {
//               val = cfd_value (s, i);
//               if (val > max) {
//                  maxi = i;
//                  max  = val;
//               }
//               if (val < min) {
//                  mini = i;
//                  min  = val;
//               }
//            }
//            s->cfd.max_mV    = max;
//            s->cfd.min_mV    = min;
//            s->cfd.max_ns    = i2ns (s->cfd.first_ns, maxi);
//            s->cfd.min_ns    = i2ns (s->cfd.first_ns, mini);
//            fprintf (stdout, "CFD info : Max_mV=%lf, min_mV=%lf, max_ns=%lf, min_ns=%lf,\n" , max, min, s->cfd.min_ns, s->cfd.max_ns);
//         }
//         return 0; // Ok
//      }
//   }
int sampling_init (sampling_t*      s,        //  return 0 when ok
                   sampler_meta_t*  meta,     //  sampler meta data (label, time shift & voltage range)
                   faster_data_p    sampler)  //  sampler as faster data
{
   unsigned short  label        = meta->label;
   unsigned short  type_alias   = meta->type_alias;
   double          before_th_ns = TO_NS * round ((-1 * meta->shift_ns) / TO_NS);
         double val;
         double qtot = 0.0;
         double max  = INT_MIN;
         double min  = INT_MAX;
         int    maxi = 0;
         int    mini = 0;
         int    i;
   sampler_range_t range_mV     = meta->range_mV;
   if ((before_th_ns < 0.0) || (before_th_ns >= SAMPLER_NB_PTS_MAX * TO_NS))
   {
      return 1;  //  error
   } else {
      s->range_mV        = range_mV;
      s->label           = label;
      s->type_alias      = type_alias;
      s->ts_ns           = faster_data_clock_ns  (sampler);
      s->raw.nb_pts      = faster_data_load_size (sampler) / 2;
      s->cfd.nb_pts      = faster_data_load_size (sampler) / 2; //TO CHANGE
      s->raw.first_ns    = -1.0 * before_th_ns;
      s->raw.last_ns     = i2ns (s->raw.first_ns, s->raw.nb_pts - 1);
      s->cfd.delay_ns    = meta->cfd_delay_ns;
      s->cfd.fraction    = meta->cfd_fraction;
      s->cfd.ts_shift_ns = 0;

      //printf ("*************************************************************************************\n");
      //printf ("Sampling Info : label=%d, type_alias=%d, TS=%lf, Nb_pts=%d, First_ns=%lf, Last_ns=%lf\n" , s->label       , s->type_alias,
      //                                                                                                    s->ts_ns       , s->raw.nb_pts,
      //                                                                                                    s->raw.first_ns, s->raw.last_ns);
      memcpy (s->raw.data, faster_data_load_p (sampler), 2 * s->raw.nb_pts);
      s->raw.data[0] = s->raw.data[1] = 0; //BUG in sampler mmns
      //fprintf (stdout, "SAMPLER VALUES\n");
      for (i = 0; i < s->raw.nb_pts; i++) {
         val = raw_value (s, i);
         //fprintf (stdout, "%d->%lf, ", i, val);
      }
      //fprintf (stdout, "\n");
      //  max, min, qtot
      {
         for (i = 0; i < s->raw.nb_pts; i++) {
            val = raw_value (s, i);
            if (val > max) {
               maxi = i;
               max  = val;
            }
            if (val < min) {
               mini = i;
               min  = val;
            }
         }
         s->raw.max_mV    = max;
         s->raw.min_mV    = min;
         s->raw.max_ns = i2ns (s->raw.first_ns, maxi);
         s->raw.min_ns = i2ns (s->raw.first_ns, mini);
         s->raw.max_ns = maxi;
         s->raw.min_ns = mini;
         s->raw.qtot_mVns = qtot;
         //fprintf (stdout, "Sampler info : Max value=%lf, min value=%lf, max sampler=%d, min sampler=%d,\n" , max, min, maxi, mini);
         max  = INT_MIN;
         min  = INT_MAX;
         if (s->cfd.fraction != 0) cfd_data (s);
         for (i = 0; i < s->cfd.nb_pts/2; i++) {
            val = cfd_value (s, i);
            if (val > max) {
               maxi = i;
               max  = val;
            }
         }
         for (i = maxi; i < (s->cfd.nb_pts-maxi); i++) {
            val = cfd_value (s, i);
            if (val < min) {
               mini = i;
               min  = val;
            }
         }
         s->cfd.max_mV    = max;
         s->cfd.min_mV    = min;
         s->cfd.max_ns = i2ns (s->raw.first_ns, maxi);
         s->cfd.min_ns = i2ns (s->raw.first_ns, mini);
         s->cfd.max_ns = maxi;
         s->cfd.min_ns = mini;
         //fprintf (stdout, "CFD info : Max value=%lf, min value =%lf, max sampler=%d, min sampler=%d,\n" , max, min, maxi, mini);
      }
      return 0; // Ok
   }
}

   void sampling_cfd_data (sampling_t const* s, short* dest, short* nb_pts) {
      memcpy (dest, s->cfd.data, 2 * s->cfd.nb_pts);
      *nb_pts = 2 * s->cfd.nb_pts;
   }

   unsigned short sampling_label (sampling_t const* s) {
      return s->label;
   }

   unsigned short sampling_type_alias (sampling_t const* s) {
      return s->type_alias;
   }


   //long double sampling_clock_ns (sampling_t const* s) {
   double sampling_clock_ns (sampling_t const* s) {
      return s->ts_ns;
   }


   unsigned short sampling_raw_nb_pts (sampling_t const* s) {
      return s->raw.nb_pts;
   }

   unsigned short sampling_cfd_nb_pts (sampling_t const* s) {
      return s->cfd.nb_pts;
   }


   double sampling_raw_first_ns (sampling_t const* s) {
      return s->raw.first_ns;
   }


   double sampling_cfd_first_ns (sampling_t const* s) {
      return s->cfd.first_ns;
   }


   double sampling_raw_last_ns (sampling_t const* s) {
      return s->raw.last_ns;
   }

   double sampling_cfd_last_ns (sampling_t const* s) {
      return s->cfd.last_ns;
   }


   double sampling_raw_value_mV (sampling_t const* s,
                                 double            at_ns) {
      return raw_value (s, ns2i (s->raw.first_ns, at_ns));
   }


   double sampling_cfd_value_mV (sampling_t const* s,
                                 double            at_ns) {
      return cfd_value (s, ns2i (s->cfd.first_ns, at_ns));
   }

   double sampling_raw_value_max_ns (sampling_t const* s){
      return s->raw.max_ns;
   }

   double sampling_cfd_value_max_ns (sampling_t const* s){
      return s->cfd.max_ns;
   }


   double sampling_raw_value_min_ns (sampling_t const* s){
      return s->raw.min_ns;
   }

   double sampling_cfd_value_min_ns (sampling_t const* s){
      return s->cfd.min_ns;
   }


   double sampling_raw_max_ns (sampling_t const* s,
                               double            from_ns,
                               double            to_ns) {
      int    ifrom = ns2i (s->raw.first_ns, from_ns);
      int    ito   = ns2i (s->raw.first_ns, to_ns  ) + 1;
      int    i     = 0;
      int    imax  = 0;
      double val   = 0.0;
      double max   = INT_MIN;
      for (i = ifrom; i < ito; i++) {
         val = raw_value (s, i);
         if (val > max) {
            max  = val;
            imax = i;
         }
      }
      return sampling_raw_to_ns (s, i);
   }


   double sampling_raw_max_mV (sampling_t const* s,
                               double            from_ns,
                               double            to_ns) {
      int    ifrom = ns2i (s->raw.first_ns, from_ns);
      int    ito   = ns2i (s->raw.first_ns, to_ns  ) + 1;
      int    i     = 0;
      double val   = 0.0;
      double max   = INT_MIN;
      for (i = ifrom; i < ito; i++) {
         val = raw_value (s, i);
         if (val > max) {
            max  = val;
         }
      }
      return max;
   }


   double sampling_raw_min_ns (sampling_t const* s,
                               double            from_ns,
                               double            to_ns) {
      int    ifrom = ns2i (s->raw.first_ns, from_ns);
      int    ito   = ns2i (s->raw.first_ns, to_ns  ) + 1;
      int    i     = 0;
      int    imin  = 0;
      double val   = 0.0;
      double min   = INT_MAX;
      for (i = ifrom; i < ito; i++) {
         val = raw_value (s, i);
         if (val < min) {
            min  = val;
            imin = i;
         }
      }
      return sampling_raw_to_ns (s, i);
   }


   double sampling_raw_min_mV (sampling_t const* s,
                               double            from_ns,
                               double            to_ns) {
      int    ifrom = ns2i (s->raw.first_ns, from_ns);
      int    ito   = ns2i (s->raw.first_ns, to_ns  ) + 1;
      int    i     = 0;
      double val   = 0.0;
      double min   = INT_MAX;
      for (i = ifrom; i < ito; i++) {
         val = raw_value (s, i);
         if (val < min) {
            min  = val;
         }
      }
      return min;
   }


   double sampling_average_mV (sampling_t const* s,
                               double            from_ns,
                               double            to_ns) {
      int ifrom  = ns2i (s->raw.first_ns, from_ns);
      int ito    = ns2i (s->raw.first_ns, to_ns  ) + 1;
      int i      = 0;
      double avg = 0.0;
      for (i = ifrom; i < ito; i++) {
         avg += raw_value (s, i);
      }
      return avg / (ito - ifrom);
   }


   double sampling_raw_charge_mVns  (sampling_t const* s,
                                     double            from_ns,
                                     double            to_ns) {
      double q     = 0.0;
      int    ifrom = sampling_raw_to_idx (s, from_ns);
      int    ito   = sampling_raw_to_idx (s, to_ns) + 1;
      int    i;
      for (i = ifrom; i < ito; i++) {
         q += raw_value (s, i) * TO_NS;
      }
      return q;
   }


   double sampling_xpoint_ns (sampling_t const* s,
                              double            xval_mV,
                              int               rising_edge,
                              double            from_ns,
                              double            to_ns,
                              int               interpol) {
      if (interpol == 1) {
         return sampling_xpoint_ns_order1 (s, xval_mV, rising_edge, from_ns, to_ns);
      } else if (interpol == 2) {
         return sampling_xpoint_ns_order2 (s, xval_mV, rising_edge, from_ns, to_ns);
      } else {
         printf ("sampling_xpoint_ns ERROR order not supported\n");
         return to_ns + 1.0;
      }
   }


   double sampling_xpoint_ns_order1 (sampling_t const* s,          //  LINEAR INPERPOLATION
                                     double            xval_mV,
                                     int               rising_edge,
                                     double            from_ns,
                                     double            to_ns) {
      double t0, v0, v1;
      int    i, j, ok;
      int    i_from = sampling_cfd_to_idx (s, from_ns);
      int    i_to   = sampling_cfd_to_idx (s, to_ns) + 1;
      i_from        = (int)from_ns;
      i_to          = (int)to_ns + 1;
      i  = i_from;
      v1 = cfd_value (s, i);
      ok = 0;
      if (rising_edge) {
         while (i < i_to) {
            v0 = v1;
            v1 = cfd_value (s, i + 1);
            if ((v0 < xval_mV) && (v1 >= xval_mV)) {
               ok = 1;
               break;
            }
            i++;
         }
      } else {
         while (i < i_to) {
            v0 = v1;
            v1 = cfd_value (s, i + 1);
            //printf ("i=%d, v0=%lf, v1=%lf xval=%lf\n", i, v0, v1, xval_mV);
            if ((v0 > xval_mV) && (v1 <= xval_mV)) {
               ok = 1;
               break;
            }
            i++;
         }
      }
      if (ok) {
         t0 = sampling_cfd_to_ns (s, i);
         //printf ("i=%d t0=%fns v0=%fmV xval=%fmV v1=%fmV ratio=%f\n", i, t0, v0, xval_mV, v1, (xval_mV - v0) / (v1 - v0));
         return t0 + TO_NS * (xval_mV - v0) / (v1 - v0);
      } else {
         return to_ns + 1.0;
      }
   }


   double sampling_xpoint_ns_order2 (sampling_t const* s,      //  INTERPOLATION POLY2
                                     double            xval_mV,
                                     int               rising_edge,
                                     double            from_ns,
                                     double            to_ns) {
      double inc = 2.0; //  ns
      double a, b, c;
      double C;
      double delta;
      double z1, z2;
      double t0, t1, v0, v1, v2;
      int    i, j, ok;
      int    i_from = sampling_cfd_to_idx (s, from_ns) - 1;
      int    i_to   = sampling_cfd_to_idx (s, to_ns)   + 1;   //  TODO i min i max !!!
      i_from        = (int)from_ns;
      i_to          = (int)to_ns + 1;
      i  = i_from;
      v1 = cfd_value (s, i);
      i++;
      v2 = cfd_value (s, i);
      ok = 0;
      if (rising_edge) {
         while (!ok && (i < i_to)) {
            v0 = v1;
            v1 = v2;
            v2 = cfd_value (s, i + 1);
            if ((v1 < xval_mV) && (v2 >= xval_mV)) {
               ok = 1;
               break;
            }
            i++;
         }
      } else {
         while (!ok && (i < i_to)) {
            v0 = v1;
            v1 = v2;
            v2 = cfd_value (s, i + 1);
            if ((v1 > xval_mV) && (v2 <= xval_mV)) {
               ok = 1;
               break;
            }
            i++;
         }
      }
      if (ok) {
         t1 = sampling_cfd_to_ns (s, i);
         t0 = t1 - inc;
         a  = (v2 - 2*v1 + v0) / (2*inc*inc);
         b  = (v1 - v0) / inc - a*(t0 + t1);
         c  = v0 - a*t0*t0 - b*t0;
         C  = c - xval_mV;
         delta = b*b - 4*a*C;
         z1    = (-b - sqrt (delta)) / (2*a);
         z2    = (-b + sqrt (delta)) / (2*a);
         return z1;
      } else {
         return to_ns + 1.0;
      }
   }


   unsigned int sampling_raw_to_idx (sampling_t const* s,
                                     double            at_ns) {
      return ns2i (s->raw.first_ns, at_ns);
   }


   unsigned int sampling_cfd_to_idx (sampling_t const* s,
                                     double            at_ns) {
      return ns2i (s->cfd.first_ns, at_ns);
   }


   double sampling_raw_to_ns (sampling_t const* s,
                              unsigned int      idx) {
      return i2ns (s->raw.first_ns, idx);
   }


   double sampling_cfd_to_ns (sampling_t const* s,
                              unsigned int      idx) {
      return i2ns (s->cfd.first_ns, idx);
   }


   double sampling_to_mV (sampling_t const* s,
                          short             rawdata) {
      return raw2mV (s->range_mV, rawdata);
   }


