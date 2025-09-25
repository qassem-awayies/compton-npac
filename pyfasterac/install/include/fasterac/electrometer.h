
//
//  ELECTROMETER data definitions
//



#ifndef ELECTROMETER_H
#define ELECTROMETER_H 1

#ifdef __cplusplus
extern "C" {
#endif


#include <math.h>
#include <string.h>
#include <stdio.h>

#include "fasterac/fasterac.h"



//  CONST

  typedef enum {
    ELECTROMETER_TYPE_ALIAS = 81
  } electrometer_const;

  static const char* ELECTROMETER_TYPE_NAME = "ELECTROMETER";


//  DATA DEFINITIONS

  typedef struct output_channel {
    unsigned charge    : 31;
    unsigned saturated :  1;
  } output_channel;


  typedef struct electrometer_data {
    unsigned int           channel_mask;
    struct output_channel  channel [32];
  } electrometer_data;



//  DATA PROC

  int    electrometer_nb_channels (struct electrometer_data elec);
         // return the number of channels present in the data (min 0, max 32)

  int    electrometer_channel_present (struct electrometer_data elec, int channel_num);
         // return 1 if channel present, else 0  (channel_num [1..32])

  int    electrometer_channel_charge_raw (struct electrometer_data elec, int channel_num);
         // return the charge of the channel in raw format,
         // or a negative value if this channel isn't present in the data  (channel_num [1..32])

  double electrometer_channel_charge_pC (struct electrometer_data elec, int channel_num);
         // return the charge of the channel in pC,
         // or a negative value if this channel isn't present in the data  (channel_num [1..32])

  int    electrometer_channel_saturated (struct electrometer_data elec, int channel_num);
         // return 1 measure is saturated, 0 not saturated, -1 channel not present. (channel_num [1..32])


//  DATA TO STRING (used by faster_disfast)

static inline void electrometer_attributes_str (faster_data_p data, char* elec_str) {
   electrometer_data elec;
   int               nb_out;
   int               satur  [33];
   double            charge [33];
   int               j;
   //
   faster_data_load (data, &elec);
   nb_out = electrometer_nb_channels (elec);                    //  nb output channels
   for (j=1; j<=32; j++) {                                      //  channels from 1 to 32
     satur [j] = electrometer_channel_saturated (elec, j);
     if (satur [j] != -1) {                                     //  -1  =>  channel 'j' not present in the data
       charge [j] = electrometer_channel_charge_pC (elec, j);   //  get charge from channel 'j' (pico Coulomb)
     }
   }
   //
                    strcpy  (elec_str, "");
                    sprintf (elec_str, "%s   Nout=%d   ", elec_str, nb_out);
   for (j=1; j<=32; j++) {
                    sprintf (elec_str, "%s[%d:%fpC",      elec_str, j, charge [j]);  //  display [chan:charge:sat]
     if (satur [j]) sprintf (elec_str, "%s:sat] ",        elec_str);
     else           sprintf (elec_str, "%s] ",            elec_str);
   }
}




#ifdef __cplusplus
}
#endif


#endif  // ELECTROMETER_H
