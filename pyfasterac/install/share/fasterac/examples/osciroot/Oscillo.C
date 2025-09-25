#include "limits.h"
#include "TMath.h"
#include "Riostream.h"


#include "Oscillo.h"


ClassImp (Oscillo)

void  Oscillo::Add (Oscillo* toAdd) {
  int n_max    = -1;
  int n_min    = -1;
  this->max_mV = INT_MIN;
  this->min_mV = INT_MAX;
  if (toAdd != NULL) {
    for (int i=0; i<nb_pts; i++) {
      this->y_mV [i] += toAdd->y_mV [i];
      if (this->y_mV [i] > this->max_mV) {
        this->max_mV = this->y_mV [i];
        n_max        = i;
      }
      if (this->y_mV [i] < this->min_mV) {
        this->min_mV = this->y_mV [i];
        n_min        = i;
      }
    }
    this->max_ns = this->num2ns (n_max);
    this->min_ns = this->num2ns (n_min);
  }
}


void Oscillo::Sub (Oscillo* toSub) {
  int n_max    = -1;
  int n_min    = -1;
  this->max_mV = INT_MIN;
  this->min_mV = INT_MAX;
  if (toSub != NULL) {
    for (int i=0; i<nb_pts; i++) {
      this->y_mV [i] -= toSub->y_mV [i];
      if (this->y_mV [i] > this->max_mV) {
        this->max_mV = this->y_mV [i];
        n_max  = i;
      }
      if (this->y_mV [i] < this->min_mV) {
        this->min_mV = this->y_mV [i];
        n_min  = i;
      }
    }
    this->max_ns = this->num2ns (n_max);
    this->min_ns = this->num2ns (n_min);
  }
}


void Oscillo::Mult (Float_t coef) {
  int n_max = -1;
  int n_min = -1;
  this->max_mV    = INT_MIN;
  this->min_mV    = INT_MAX;
  if (coef != 1.0) {
    for (int i=0; i<this->nb_pts; i++) {
      this->y_mV [i] *= coef;
      if (this->y_mV [i] > this->max_mV) {
        this->max_mV = this->y_mV [i];
        n_max  = i;
      }
      if (this->y_mV [i] < this->min_mV) {
        this->min_mV = this->y_mV [i];
        n_min  = i;
      }
    }
    this->max_ns = this->num2ns (n_max);
    this->min_ns = this->num2ns (n_min);
  }
}


void Oscillo::Init (short*  pts,
                    Int_t   nb_pts,
                    Float_t x0_ns,
                    Float_t n2ns,
                    Float_t n2mV) {
   this->Props_Init  (nb_pts, x0_ns, n2ns, n2mV);
   this->Points_Init (pts);
}



void Oscillo::Props_Init   (Int_t    nb_pts,
                            Float_t  x0_ns,
                            Float_t  n2ns,
                            Float_t  n2mV) {
  this->nb_pts = nb_pts;
  this->x0_ns  = x0_ns;
  this->n2ns   = n2ns;
  this->n2mV   = n2mV;
}



void Oscillo::Points_Init  (short* pts) {
  int n_max    = -1;
  int n_min    = -1;
  this->max_mV = INT_MIN;
  this->min_mV = INT_MAX;
  if (pts == NULL) {
    for (int i=0; i<this->nb_pts; i++) this->y_mV [i] = 0.0;
    n_max = n_min = 0;
  } else {
    for (int i=0; i<this->nb_pts; i++) {
      this->y_mV [i] = pts [i] * this->n2mV;
      if (this->y_mV [i] > this->max_mV) {
        this->max_mV = this->y_mV [i];
        n_max  = i;
      }
      if (this->y_mV [i] < this->min_mV) {
        this->min_mV = this->y_mV [i];
        n_min  = i;
      }
    }
  }
  this->max_ns = this->num2ns (n_max);
  this->min_ns = this->num2ns (n_min);
}



Float_t Oscillo::Baseline (Float_t width_ns) {
  int     n  = this->ns2num (width_ns);
  Float_t bl = 0.0;
  for (int i=0; i<n; i++) {
    bl += this->y_mV [i];
  }
  return bl / n;
}


Float_t Oscillo::Charge_mVns (Float_t t1_ns, Float_t t2_ns) {
  int     n1 = this->ns2num (t1_ns);
  int     n2 = this->ns2num (t2_ns);
  Float_t q  = 0.0;
  for (int i=n1; i<n2; i++) {
    q += this->y_mV [i] * this->n2ns;
  }
  return q;
}


void Oscillo::Fill (TH1F* histo) {
  histo->Reset ();
  for (int i=0; i<this->nb_pts; i++) {
    //  histo->Fill (this->x0_ns + i * this->n2ns, this->y_mV [i]);
    histo->SetBinContent (i, this->y_mV [i]);
  }
  Float_t dy =      (this->max_mV - this->min_mV) / 10;
  histo->SetMinimum (this->min_mV - dy);
  histo->SetMaximum (this->max_mV + dy);
}


Float_t Oscillo::num2ns (int n) {
  return this->x0_ns + n * this->n2ns;
}


int Oscillo::ns2num (Float_t t_ns) {
  return (t_ns - this->x0_ns) / this->n2ns;
}

