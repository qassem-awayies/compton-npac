
#ifndef Discrillo_h
#define Discrillo_h


#include "Oscillo.h"


//  Oscillo inheritage demo class
//  with specific values dedicated to the example oscillo14_10k.fast (see Discrillo.C)


class Discrillo : public Oscillo	{

	public:
    Bool_t Gamma   ();
    Bool_t Neutron ();
    Bool_t Unknown ();
    Int_t  What    (); // 0 : unknown
                       // 1 : gamma
                       // 2 : neutron

  public :
    ClassDef (Discrillo, 2)

};

#endif
