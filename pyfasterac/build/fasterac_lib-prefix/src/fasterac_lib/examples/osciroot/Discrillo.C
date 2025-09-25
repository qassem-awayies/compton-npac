#include "TMath.h"
#include "Riostream.h"


#include "Discrillo.h"

  //
  // specific values fitted to the given example (oscillo14_10k.fast)
  //
#define  T1_14  -14.0   // ns       charge window limits
#define  T2_14   14.0   // ns       ...
#define  T3_14  600.0   // ns       ...
#define RAT_14    0.25  //          discrimination
#define TOT_14 1000.0   // mV.ns    charge mini


ClassImp (Discrillo)


Bool_t  Discrillo::Gamma () {
  if (this->What () == 1) return kTRUE;
  return kFALSE;
}

Bool_t  Discrillo::Neutron () {
  if (this->What () == 2) return kTRUE;
  return kFALSE;
}

Bool_t  Discrillo::Unknown () {
  if (this->What () == 0) return kTRUE;
  return kFALSE;
}

Int_t  Discrillo::What () {
  Float_t fast  = this->Charge_mVns (T1_14, T2_14);
  Float_t slow  = this->Charge_mVns (T2_14, T3_14);
  Float_t tot   = fast + slow;
  Float_t ratio = slow / tot;
  if (tot > TOT_14) {
    if ((ratio > 0) and (ratio < RAT_14)) return 1;
    else                                  return 2;
  }
  return 0;
}

