
#include <math.h>
#include "fasterac/fast_data.h"
#include "fasterac/group.h"



//  COUNTER tools
int group_counter_set_value  (faster_data_p data, unsigned int multiplicity, unsigned int delta_time)
{
   group_counter* grp_counter;

   grp_counter          = (group_counter*) faster_data_load_p (data);
   grp_counter->mult    = multiplicity;
   grp_counter->delta_t = delta_time;
   
   return 0;
}	

