

#include "fasterac/qtdc.h"


double to_mV (int a) {
  //return a * 0.004558563;             //   CARAS VOLTAGE  LSB : 2*2390 / 2^20 mV  //  14 bits signés
  return a * 0.291748046875;             //   CARAS VOLTAGE  LSB : 2*2390 / 2^20 mV
}

double to_mVns (int q) {
  return to_mV (q) * 2.0;       //   CARAS VOLTAGE  &  SAMPLING 2.0 ns
}

double to_ns  (int dt) {
  return dt * 7.8125e-3;              //   HR CLOCK LSB : 2.0 / 256 ns
}


double qtdc_tdc_ns (qtdc q) {
  return to_ns (q.head.tdc);
}


double qtdc_tdc_sec (qtdc q) {
  return qtdc_tdc_ns (q) * 1.0e-9;
}

int qtdc_pileup (qtdc q) {
  return q.head.pileup;
}

int qtdc_saturated (qtdc q) {
  return q.head.saturated;
}

int qtdc_nb_q (qtdc q) {
  return q.head.nb_q;
}

int qtdc_charge_raw (qtdc q, int n) {
  return q.data [n-1];
}

double qtdc_charge_mVns (qtdc q, int n) {
  return to_mVns (q.data [n-1]);
}

int qtdc_t2t_charge_ok (qtdc q) {
  return q.head.t2t_q;
}

int qtdc_t2t_charge_raw (qtdc q) {
  return qtdc_charge_raw (q, q.head.nb_q + 1);
}

double qtdc_t2t_charge_mVns (qtdc q) {
  return qtdc_charge_mVns (q, q.head.nb_q + 1);
}

double qtdc_t2t_width_ns (qtdc q) {
  return q.head.t2t_width * 2.0;
}

int qtdc_t2t_max_ok (qtdc q) {
  return q.head.t2t_max;
}

int qtdc_t2t_max_raw (qtdc q) {
  struct qtdc_max max;
  int idx = q.head.nb_q;
  if (q.head.t2t_q) idx++;
  max = *(struct qtdc_max*)& q.data [idx];
  return max.a;
}

double qtdc_t2t_max_mV (qtdc q) {
  return to_mV (qtdc_t2t_max_raw (q));
}

double qtdc_t2t_max_pos_ns (qtdc q) {
  struct qtdc_max max;
  int idx = q.head.nb_q;
  if (q.head.t2t_q) idx++;
  max = *(struct qtdc_max*)& q.data [idx];
  return max.t * 2.0;
}





