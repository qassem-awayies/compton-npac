
#include "fasterac/plas.h"

short faster_data_plas_nb_pts (faster_data_p data) {
   short lsize = 0;
   short nbpts = 0;
   if (faster_data_type_alias (data) == PLAS_RAW_DATA_TYPE_ALIAS) {
      lsize = faster_data_load_size (data);
      nbpts = 242;
   }
   return nbpts;
}
