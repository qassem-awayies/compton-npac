
//
//  ONLINE TOOLS 
//    => remote faster_gui
//    => pipeline
//


#ifndef FASTERAC_ONLINE_H
#define FASTERAC_ONLINE_H 1

#ifdef __cplusplus
extern "C" {
#endif

   int faster_gui_start (char* ip, unsigned short port, char* filename);

   int faster_gui_stop  (char* ip, unsigned short port);

#ifdef __cplusplus
}
#endif


#endif  // FASTERAC_ONLINE_H
