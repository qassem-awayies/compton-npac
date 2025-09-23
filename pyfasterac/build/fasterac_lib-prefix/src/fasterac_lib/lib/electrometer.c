

#include "fasterac/electrometer.h"


#define LSB_PICOCOULOMB 45.777e-6   // range 3pC 16bits


int electrometer_nb_channels (struct electrometer_data elec) {
  int          i;
  unsigned int one_bit = 1;
  int          n       = 0;
  for (i=0; i<32; i++) {
    if (one_bit & elec.channel_mask) n = n + 1;
    one_bit = one_bit << 1;
  }
  return n;
}


int electrometer_channel_present (struct electrometer_data elec, int channel_num) {
  int          present = 0;
  unsigned int one_bit = 1;
  if (channel_num >= 1 && channel_num <= 32) {
    one_bit = one_bit << (channel_num - 1);
    if (one_bit & elec.channel_mask) present = 1;
  }
  return present;
}


int electrometer_channel_charge_raw (struct electrometer_data elec, int channel_num) {
  int          charge  = -1;
  unsigned int one_bit =  1;
  int          data_n  =  0;
  int          i;
  if (electrometer_channel_present (elec, channel_num)) {
    for (i=1; i<channel_num; i++) {
      if (one_bit & elec.channel_mask) data_n = data_n + 1;
      one_bit = one_bit << 1;
    }
    charge = elec.channel [data_n].charge;
  }
  return charge;
}


double electrometer_channel_charge_pC (struct electrometer_data elec, int channel_num) {
  double charge = electrometer_channel_charge_raw (elec, channel_num);
  if (charge < 0.0) return charge;
  else              return charge * LSB_PICOCOULOMB;
}


int electrometer_channel_saturated (struct electrometer_data elec, int channel_num) {
  int          saturated = -1;
  unsigned int one_bit   =  1;
  int          data_n    =  0;
  int          i;
  if (electrometer_channel_present (elec, channel_num)) {
    for (i=1; i<channel_num; i++) {
      if (one_bit & elec.channel_mask) data_n = data_n + 1;
      one_bit = one_bit << 1;
    }
    saturated = elec.channel [data_n].saturated;
  }
  return saturated;
}


