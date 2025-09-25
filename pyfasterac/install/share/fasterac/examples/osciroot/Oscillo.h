
#ifndef Oscillo_h
#define Oscillo_h

#include "TNamed.h"
#include "TH1.h"

#include "fasterac/fast_data.h"


class Oscillo : public TNamed	{


	public:
    Float_t y_mV [OSCILLO_NB_PTS_MAX];
    Int_t   nb_pts;
    Float_t max_mV;
    Float_t max_ns;
    Float_t min_mV;
    Float_t min_ns;
    Float_t x0_ns;
    Float_t n2ns;
    Float_t n2mV;

	public:
    //
                      Oscillo ()           : TNamed ("AnOscillo", "AnOscillo") {};
                      Oscillo (char* name) : TNamed (name, name)               {};
    //
    virtual void      Init         (short*   pts, Int_t nb_pts, Float_t x0_ns, Float_t n2ns, Float_t n2mV);
    virtual void      Props_Init   (Int_t    nb_pts,
                                    Float_t  x0_ns,
                                    Float_t  n2ns,
                                    Float_t  n2mV);
    virtual void      Points_Init  (short*   pts);
    //
    virtual void      Add          (Oscillo* toAdd);
    virtual void      Sub          (Oscillo* toSub);
    virtual void      Mult         (Float_t  coef);
    virtual void      Fill         (TH1F*    histo);
    virtual Float_t   Baseline     (Float_t  width_ns);
    virtual Float_t   Charge_mVns  (Float_t  t1_ns, Float_t t2_ns);

	private:
    Float_t num2ns (int n);
    int     ns2num (Float_t t_ns);

  public :
    ClassDef (Oscillo, 2)

};

#endif
