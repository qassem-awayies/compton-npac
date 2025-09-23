
//
//  Sampling 'object' encapsulating a CARAS sampler data
//
//      sam_tpler_data is raw data
//         => low level infos
//
//      sampling_p is a sampler including range and trigger infos
//         => higher level infos
//         => kind of oriented object
//

#ifndef SAMPLING_H
#define SAMPLING_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>

#include "fasterac/fasterac.h"
#include "fasterac/sampler.h"
#include "fasterac/qdc.h"


   //  type sampler_meta_t

   typedef struct sampler_meta_t {
      unsigned short    label;
      unsigned short    type_alias;
      double            shift_ns;
      unsigned short    cfd_fraction;
      unsigned short    cfd_delay_ns;
      sampler_range_t   range_mV;
   } sampler_meta_t;

   //  type sampling_raw_t
   typedef struct sampler_raw_t {
      //  DATA
      short             data [SAMPLER_NB_PTS_MAX];
      unsigned short    nb_pts;
      //  USEFULL VALUES
      double            max_mV;   //  overall max, min and charge
      double            min_mV;
      double            qtot_mVns;
      //  TIMES RELATIVE TO TRIGGER (0 ns)
      double            first_ns;
      double            last_ns;
      double            max_ns;
      double            min_ns;
   } sampler_raw_t;

   //  type sampling_cfd_t
   typedef struct sampler_cfd_t {
      unsigned short    fraction;
      unsigned short    delay_ns;
      //  DATA
      short             data [SAMPLER_NB_PTS_MAX];
      unsigned short    nb_pts;
      //  USEFULL VALUES
      double            max_mV;   //  overall max, min and charge
      double            min_mV;
      //  TIMES RELATIVE TO TRIGGER (0 ns)
      double            first_ns;
      double            last_ns;
      double            max_ns;
      double            min_ns;
      double            ts_shift_ns;
   } sampler_cfd_t;

   //  type sampling_t
   typedef struct sampling_t {
      //  BASICS
      unsigned short    label;
      unsigned short    type_alias;
      double            ts_ns;       // TimeStamp in ns
      //  DATA
      sampler_range_t   range_mV;
      sampler_raw_t     raw;
      sampler_cfd_t     cfd;
   } sampling_t;


   //  initialization from a sampler_data (as faster_data)
   //  with daq conditions (label, range and trigger shift)

   int               sampling_init         (sampling_t*       s,            //  return 0 when ok
                                            sampler_meta_t*   meta,
                                            faster_data_p     sampler);

   //  data basics

   unsigned short    sampling_label        (sampling_t const* s);

   unsigned short    sampling_type_alias   (sampling_t const* s);

   double            sampling_clock_ns     (sampling_t const* s);           // time stamp (lsb = 2ns)

   unsigned short    sampling_raw_nb_pts   (sampling_t const* s);

   unsigned short    sampling_cfd_nb_pts   (sampling_t const* s);

   //  usefull values

   double            sampling_raw_first_ns     (sampling_t const* s);

   double            sampling_cfd_first_ns     (sampling_t const* s);

   double            sampling_raw_last_ns      (sampling_t const* s);

   double            sampling_cfd_last_ns      (sampling_t const* s);

   double            sampling_raw_value_mV     (sampling_t const* s,
                                                double            at_ns);       //  first_ns <= at_ns <= last_ns

   double            sampling_raw_value_mV     (sampling_t const* s,
                                                double            at_ns);       //  first_ns <= at_ns <= last_ns

   double            sampling_raw_value_max_ns (sampling_t const* s);           //  returns the position of the value max of the data (ns)

   double            sampling_cfd_value_max_ns (sampling_t const* s);           //  returns the position of the value max of the data (ns)

   double            sampling_raw_value_min_ns (sampling_t const* s);           //  returns the position of the value min of the data (ns)

   double            sampling_cfd_value_min_ns (sampling_t const* s);           //  returns the position of the value min of the data (ns)

   double            sampling_raw_max_ns       (sampling_t const* s,            //  returns the position of the value max (ns)
                                                double            from_ns,      //  between from_ns and to_ns
                                                double            to_ns);

   double            sampling_raw_max_mV       (sampling_t const* s,            //  returns value max (mV)
                                                double            from_ns,      //  between from_ns and to_ns
                                                double            to_ns);

   double            sampling_raw_min_ns       (sampling_t const* s,            //  returns the position of the value min (ns)
                                                double            from_ns,      //  ...
                                                double            to_ns);

   double            sampling_raw_min_mV       (sampling_t const* s,            //  returns value min (mV)
                                                double            from_ns,      //
                                                double            to_ns);

   double            sampling_average_mV       (sampling_t const* s,            //  returns the average value (mV)
                                                double            from_ns,      //
                                                double            to_ns);

   double            sampling_raw_charge_mVns  (sampling_t const* s,            //  returns the charge value (mVns)
                                                double            from_ns,
                                                double            to_ns);

   double            sampling_xpoint_ns    (sampling_t const* s,            //  returns the crossing point at xval_mV
                                            double            xval_mV,      //  this time is referenced by the 0ns of the sampling (acquisition threshold)
                                            int               rising_edge,  //  direction of crossing (1 -> rising edge | 0 -> falling edge)
                                            double            from_ns,      //  start of crossing search
                                            double            to_ns,        //  stop of crossing search
                                            int               interpol);    //  order of interpolation [1 | 2]
                                                                            //  returns a value after 'to_ns' if there is no crossing between from and to
   //  conversions

   unsigned int      sampling_raw_to_idx   (sampling_t const* s,
                                            double            at_ns);       //  first_ns <= at_ns <= last_ns

   unsigned int      sampling_cfd_to_idx   (sampling_t const* s,
                                            double            at_ns);       //  first_ns <= at_ns <= last_ns

   double            sampling_raw_to_ns    (sampling_t const* s,
                                            unsigned int      idx);         //  0 <= idx <= nb_pts - 1

   double            sampling_cfd_to_ns    (sampling_t const* s,
                                            unsigned int      idx);         //  0 <= idx <= nb_pts - 1

   double            sampling_to_mV        (sampling_t const* s,
                                            short             rawdata);

   void              sampling_cfd_data     (sampling_t const* s,
                                            short*            dest,
                                            short*            nb_pts);
   //

#ifdef __cplusplus
}
#endif


#endif  // SAMPLING_H
