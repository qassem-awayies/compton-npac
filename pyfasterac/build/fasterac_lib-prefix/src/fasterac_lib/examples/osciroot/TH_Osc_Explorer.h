
#ifndef TH_Osc_Explorer_h
#define TH_Osc_Explorer_h

#include <TROOT.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TMarker.h>
#include <TPaveText.h>

#include "Oscillo.h"


class TH_Osc_Explorer {


  public :

    //  Monodim Ctor
    TH_Osc_Explorer (TTree*   osc_tree,                                                  // TTree with Oscillo* events
                     Oscillo* osc,                                                       // Oscillo* instance
                     void (*oscillo_get_X) (Oscillo* osc, Bool_t* select, Double_t* x),  // procedure on one Oscillo* : get X if selected
                     const char* title = "Oscillo explorer",                             // title of monodim
                     Int_t nb_osc_max  = -1,                                             // nb data limitation (-1 = no limit)
                     Int_t nbinsx = 100, Double_t xlow = 0, Double_t xup = -1);          // TH1F input params

    //  Bidim Ctor
    TH_Osc_Explorer (TTree*   osc_tree,
                     Oscillo* osc,
                     void (*oscillo_get_XY) (Oscillo* osc, Bool_t* select, Double_t* x, Double_t* y),  //  2D
                     const char* title = "Oscillo explorer",
                     Int_t nb_osc_max  = -1,
                     Int_t nbinsx = 100, Double_t xlow = 0, Double_t xup = -1,
                     Int_t nbinsy = 100, Double_t ylow = 0, Double_t yup = -1);                        //  2D

    //  copy & =op todo

    virtual ~TH_Osc_Explorer ();

    virtual void SetLogy (Bool_t logy = kTRUE);
    virtual void SetLogz (Bool_t logy = kTRUE);
    virtual void SetColz (Bool_t colz = kTRUE);

    class TreeEntry {
      public :
        Int_t   entry;
        Float_t x;
        Float_t y;
    };


  protected :
    Bool_t     bidim;
    TTree*     tree;
    TreeEntry* entries;
    int        nb_max;
    int        nb_entries;
    Oscillo*   cur_osc;
    int        cur_sel;
    int        first_sel;
    int        last_sel;
    TPad*      top;
    TH1*       histo;
    Int_t      nb_x_bins;
    TH1F*      histo_osc;
    TMarker*   histo_pos1;
    TMarker*   histo_pos2;
    TPaveText* info1;
    TPaveText* info2;
    void (*osc_get_X)  (Oscillo* osc, Bool_t* select, Double_t* x);
    void (*osc_get_XY) (Oscillo* osc, Bool_t* select, Double_t* x, Double_t* y);


  protected :
    virtual void Part1_Ctor    (TTree* osc_tree, Oscillo* osc, Int_t nb_osc_max);
    virtual void Part2_1D_Ctor (const char* title,
                                Int_t nbinsx, Double_t xlow, Double_t xup,
                                void (*oscillo_get_X) (Oscillo* osc, Bool_t* select, Double_t* x));
    virtual void Part2_2D_Ctor (const char* title,
                                Int_t nbinsx, Double_t xlow, Double_t xup,
                                Int_t nbinsy, Double_t ylow, Double_t yup,
                                void (*oscillo_get_XY) (Oscillo* osc, Bool_t* select, Double_t* x, Double_t* y));
    virtual void Part3_Ctor    (Oscillo* osc);
    virtual void Selection     (Double_t xx, Double_t yy);
    virtual void Display_Next  ();
    static  int  TreeEntry_X_Compare (const void* te1, const void* te2);
    static  int  TreeEntry_Y_Compare (const void* te1, const void* te2);

  public :
    virtual void Click_Selection ();
    virtual void Click_Next      ();

  public :
    ClassDef (TH_Osc_Explorer, 0);

};

#endif
