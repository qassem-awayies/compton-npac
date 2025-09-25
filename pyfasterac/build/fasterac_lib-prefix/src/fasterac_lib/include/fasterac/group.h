
#ifndef GROUP_H
#define GROUP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "fasterac/fast_data.h"
#include <stdio.h>

typedef enum {
  GROUP_TYPE_ALIAS         =  10,
  // GROUP_COUNTER_TYPE_ALIAS =  30,
  GROUP_COUNTER_TYPE_ALIAS =  30,
} group_const;

typedef struct group_counter {
  unsigned mult    : 32;
  unsigned delta_t : 32;
} group_counter;


static const char* GROUP_TYPE_NAME         = "GROUP";
static const char* GROUP_COUNTER_TYPE_NAME = "GROUP_COUNTER";

static size_t GROUP_COUNTER_TYPE_SIZE = sizeof (group_counter);

int group_counter_set_value  (faster_data_p data, unsigned int multiplicity, unsigned int delta_time);

//  DATA TO STRING (used by faster_disfast)

static inline void group_attributes_str (faster_data_p data, char* group_str) {     // for documentation purpose :
   unsigned char          alias;                                                    // this is not the one called in faster_disfast
   unsigned short         label;
   unsigned short         lsize;
   unsigned long long     clock;
   faster_buffer_reader_p group_reader;
   void*                  group_buffer;
   faster_data_p          group_data;
   int                    group_n = 0;
   alias = faster_data_type_alias (data);
   label = faster_data_label      (data);
   lsize = faster_data_load_size  (data);
   clock = faster_data_clock_ns   (data);
   sprintf (group_str, "%15s %5d  %lld ns\n", GROUP_TYPE_NAME, label, clock);
   if (alias == GROUP_TYPE_ALIAS) {
      group_buffer = faster_data_load_p (data);
      group_reader = faster_buffer_reader_open (group_buffer, lsize);
      sprintf (group_str, "%s   ", group_str);
      sprintf (group_str, "%s           -------------------------------------------\n", group_str);
      while ((group_data = faster_buffer_reader_next (group_reader)) != NULL) {
         group_n += 1;
         alias = faster_data_type_alias (group_data);
         label = faster_data_label      (group_data);
         clock = faster_data_clock_ns   (group_data);
         sprintf (group_str, "%s   ", group_str);
         sprintf (group_str, "%s   ", group_str);
         sprintf (group_str, "%s     %d type=%d label=%d clock=%lldns\n", group_str, group_n, alias, label, clock);
      }
      sprintf (group_str, "%s   ", group_str);
      sprintf (group_str, "%s           -------------------------------------------\n", group_str);
      faster_buffer_reader_close (group_reader);
   }
}

static inline void group_counter_attributes_str (faster_data_p data, char* count_str) {
   group_counter count;
   char          text [100];

   faster_data_load (data, &count);

   sprintf (text     , " mult=%d" , count.mult     );
   sprintf (count_str, " %-15s"   , text           );
   sprintf (text     , " dift=%d", count.delta_t  );
   sprintf (count_str, "%s  %-15s", count_str, text);
}

#ifdef __cplusplus
}
#endif


#endif  // FAST_GROUP_H
