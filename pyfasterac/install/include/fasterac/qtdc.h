
//
//  QTDC CARAS data definitions
//



#ifndef QTDC_H
#define QTDC_H 1

#ifdef __cplusplus
extern "C" {
#endif

  #include <string.h>
  #include <stdio.h>

  #include "fasterac/fasterac.h"


  typedef enum {
    QTDC_TYPE_ALIAS         = 45,
    QTDC_COUNTER_TYPE_ALIAS = 51
  } qtdc_const;

  static const char* QTDC_TYPE_NAME         = "QTDC";
  static const char* QTDC_COUNTER_TYPE_NAME = "QTDC_COUNT";


  typedef struct qtdc_counter {
    unsigned trig : 32;
    unsigned calc : 32;
    unsigned sent : 32;
  } qtdc_counter;


  typedef struct qtdc_header {
    unsigned t2t_width : 16;
    signed   tdc       :  9;
    unsigned pileup    :  1;
    unsigned saturated :  1;
    unsigned nb_q      :  3;   // nb_q -> 2bits -> nb_q max = 3  ==>  pour ajouter un bool barycentre
    unsigned t2t_q     :  1;
    unsigned t2t_max   :  1;
  } qtdc_header;


  typedef struct qtdc_max {
    unsigned t   : 16;
    signed   a   : 14;
    unsigned pad : 2;
  } qtdc_max;


  typedef struct qtdc {
    struct qtdc_header head;
    int                data [6];  // Q1 Q2 Q3 Q4 t2tQ t2tMax   (nb data compris entre 0 et 6 suivant le header)
  } qtdc;


  //  Data field getters
  double qtdc_tdc_sec         (qtdc q);
  double qtdc_tdc_ns          (qtdc q);
  int    qtdc_pileup          (qtdc q);
  int    qtdc_saturated       (qtdc q);
  double qtdc_t2t_width_ns    (qtdc q);
  int    qtdc_nb_q            (qtdc q);
  double qtdc_charge_mVns     (qtdc q, int n);
  int    qtdc_t2t_charge_ok   (qtdc q);
  double qtdc_t2t_charge_mVns (qtdc q);
  int    qtdc_t2t_max_ok      (qtdc q);
  double qtdc_t2t_max_mV      (qtdc q);
  double qtdc_t2t_max_pos_ns  (qtdc q);

  int    qtdc_charge_raw      (qtdc q, int n);
  int    qtdc_t2t_charge_raw  (qtdc q);
  int    qtdc_t2t_max_raw     (qtdc q);


  //  DATA TO STRING (used by faster_disfast)

  static inline void qtdc_attributes_str (faster_data_p data, char* qt_str) {
    qtdc qt;
    int  j;
    faster_data_load (data, &qt);
    sprintf (qt_str, "  nb_q=%d", qtdc_nb_q (qt));
    if (qtdc_saturated (qt))  {
      sprintf (qt_str, "%s  saturated", qt_str);
    }
    if (qtdc_pileup (qt)) {
      sprintf (qt_str, "%s  pileup", qt_str);
    }
    sprintf (qt_str, "%s  width=%.2fns", qt_str, qtdc_t2t_width_ns (qt));
    if (qtdc_t2t_max_ok (qt)) {
      sprintf (qt_str, "%s  max=%d  max_pos=%.2fns", qt_str, qtdc_t2t_max_raw (qt), qtdc_t2t_max_pos_ns (qt));
    }
    if (qtdc_t2t_charge_ok (qt)) {
      sprintf (qt_str, "%s  t2t_q=%d", qt_str, qtdc_t2t_charge_raw (qt));
    }
    for (j=1; j<=qtdc_nb_q (qt); j++) {
      sprintf (qt_str, "%s  q%d=%d", qt_str, j, qtdc_charge_raw (qt, j));
    }
  }


  static inline void qtdc_counter_attributes_str (faster_data_p data, char* count_str) {
     qtdc_counter count;
     faster_data_load (data, &count);
     sprintf (count_str, "  trig=%d  calc=%d  sent=%d", count.trig, count.calc, count.sent);
  }



#ifdef __cplusplus
}
#endif


#endif  // QDC_CARAS_H
