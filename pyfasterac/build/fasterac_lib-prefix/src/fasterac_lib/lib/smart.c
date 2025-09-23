
#include "fasterac/smart.h"

//  get the smart timestamp
unsigned long long smart_get_ts_smart (faster_data_p data) {
   smart*        s;
   unsigned long long* p;
   unsigned char alias = faster_data_type_alias (data);

   if (alias == SMART_TYPE_ALIAS) {
      s = (smart*) faster_data_load_p (data);
      p =  (unsigned long long*)&s->ts_smart[0];
      //printf ("TS smart= %llu ", (*p >> 16) * 2 );
      return  ((*p >>16) *2);
   }
   return 0;
}


//  get the faster timestamp
unsigned long long smart_get_ts_faster (faster_data_p data) {
   smart*        s;
   unsigned long long* p;
   unsigned char alias = faster_data_type_alias (data);

   if (alias == SMART_TYPE_ALIAS) {
      s = (smart*) faster_data_load_p (data);
      p =  (unsigned long long*)&s->ts_faster[0];
      //printf ("TSfaster = %llu ", (*p >>16) *2);
      return ((*p >>16) *2);
   }
   return 0;
}




//unsigned long long faster_data_clock_ns (const faster_data_p data) {   // faster clock    48bits
//  return ((*((unsigned long long*) data)) >> 16) * FASTER_TICK_NS;     // long long       64bits
//}

