/*
 *  Faster data file  =>  CRRC4_spectro data list with statistics
 *
 */

#include <stdio.h>
#include <limits.h>

#include "fasterac/fasterac.h"
#include "fasterac/spectro.h"


//-------------------------------------------------------------------------------------------------//

struct adc_info_t {                     //  CRRC4_spectro data infos structured as list
  unsigned short     label;
  int                adc_min;
  int                adc_max;
  int                nb_data;
  int                nb_saturated;
  int                nb_pileup;
  struct adc_info_t* prec;
  struct adc_info_t* succ;
} adc_info_t;

struct adc_info_t* adc_info_new     (unsigned short label, crrc4_spectro adc);  //  create a list with a first ADC data
void               adc_info_free    (struct adc_info_t* list);                  //  free the list
struct adc_info_t* adc_info_first   (struct adc_info_t* list);                  //  get the first element
struct adc_info_t* adc_info_last    (struct adc_info_t* list);                  //  get the last element
void               adc_info_add_adc (struct adc_info_t* list, unsigned short label, crrc4_spectro adc);
                                                                                //  add infos from an ADC data

//--------------------------------------------------------------------------------//


int main (int argc, char** argv) {

  faster_file_reader_p  reader;
  faster_data_p         data;
  unsigned char         alias;
  unsigned short        label;
  crrc4_spectro         adc;
  struct adc_info_t*    info_list = NULL;
  struct adc_info_t*    info_iter = NULL;

  //  Command line usage
  if (argc != 2) {
    printf ("\n");
    printf ("  %s  :  display crrc4_spectro data informations from Faster file.\n", argv [0]);
    printf ("\n");
    printf ("  usage : \n");
    printf ("          %s  datafile.fast\n", argv[0]);
    printf ("\n");
    printf ("  ** this program doesn't treat data inside groups **\n");
    printf ("\n");
    return EXIT_SUCCESS;
  }
  //  Input file reader
  reader = faster_file_reader_open (argv  [1]);
  if (reader == NULL) {
    printf ("error opening file %s\n", argv [1]);
    return EXIT_FAILURE;
  }

  //  Loop on data
  while ((data = faster_file_reader_next (reader)) != NULL) {
    alias = faster_data_type_alias (data);
    if (alias == CRRC4_SPECTRO_TYPE_ALIAS) {
      label = faster_data_label (data);
      faster_data_load (data, &adc);
      if (info_list == NULL) {
        info_list = adc_info_new (label, adc);
      }
      adc_info_add_adc (info_list, label, adc);
    }
  }

  //  Reader close
  faster_file_reader_close (reader);

  //  Display ADC infos
  if (info_list == NULL) {
    printf ("\n");
    printf ("\n");
    printf ("               There is no crrc4_spectro data in '%s'\n", argv [1]);
    printf ("\n");
  } else {
    printf ("\n");
    printf ("\n");
    printf ("               ADC data in '%s'\n", argv [1]);
    printf ("\n");
    printf ("  -----------------------------------------------------------------------------\n");
    printf (" |    label   |  min value |  max value |  nb satur  |  nb pileup |   nb data  |\n");
    printf (" |------------|------------|------------|------------|------------|------------|\n");
    info_iter = adc_info_first (info_list);
    while (info_iter != NULL) {
      printf (" | %10d | %10d | %10d | %10d | %10d | %10d |\n", info_iter->label,
                                                                info_iter->adc_min,
                                                                info_iter->adc_max,
                                                                info_iter->nb_saturated,
                                                                info_iter->nb_pileup,
                                                                info_iter->nb_data);
      info_iter = info_iter->succ;
    }
    printf ("  -----------------------------------------------------------------------------\n");
    printf ("\n");
  }

  //  Free allocated memory
  adc_info_free (info_list);

  return EXIT_SUCCESS;

}





//-------------------------------------------------------------------------------------------------//

struct adc_info_t* adc_info_new (unsigned short label, crrc4_spectro adc) {
  struct adc_info_t* ninfo = (struct adc_info_t*) malloc (sizeof (struct adc_info_t));
  ninfo->label        = label;
  ninfo->adc_min      = adc.measure;
  ninfo->adc_max      = adc.measure;
  ninfo->nb_data      = 0;
  ninfo->nb_saturated = 0;
  ninfo->nb_pileup    = 0;
  ninfo->prec         = NULL;
  ninfo->succ         = NULL;
  if (adc.saturated) ninfo->nb_saturated = 1;
  if (adc.pileup)    ninfo->nb_pileup    = 1;
  return ninfo;
}

void adc_info_free (struct adc_info_t* list) {
  struct adc_info_t* tmp;
  list = adc_info_first (list);
  while (list != NULL) {
    tmp = list->succ;
    free (list);
    list = tmp;
  }
}

struct adc_info_t* adc_info_first (struct adc_info_t* list) {
  if (list != NULL) while (list->prec != NULL) list = list->prec;
  return list;
}

struct adc_info_t* adc_info_last (struct adc_info_t* list) {
  if (list != NULL) while (list->succ != NULL) list = list->succ;
  return list;
}

void adc_info_add_adc (struct adc_info_t* list, unsigned short label, crrc4_spectro adc) {
  struct adc_info_t* current = adc_info_first (list);
  struct adc_info_t* ninfo;
  while (current != NULL) {
    if (label < current->label) {
      ninfo         = adc_info_new (label, adc);
      ninfo->succ   = current;
      ninfo->prec   = current->prec;
      current->prec = ninfo;
      if (ninfo->prec != NULL) ninfo->prec->succ = ninfo;
      break;
    } else if (label == current->label) {
      if ((!adc.saturated) && (!adc.pileup)) {
        if ((adc.measure < current->adc_min) && (adc.measure >= 0))
                                            current->adc_min = adc.measure;
        if (adc.measure > current->adc_max) current->adc_max = adc.measure;
      } else {
        if (adc.saturated)                  current->nb_saturated++;
        if (adc.pileup)                     current->nb_pileup++;
      }
      /**/                                  current->nb_data++;
      break;
    } else if (current->succ == NULL) {
        ninfo         = adc_info_new (label, adc);
        ninfo->prec   = current;
        current->succ = ninfo;
        break;
    }
    current = current->succ;
  }
}

//-------------------------------------------------------------------------------------------------//
