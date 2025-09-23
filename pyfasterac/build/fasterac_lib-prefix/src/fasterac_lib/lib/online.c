#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fasterac/online.h"

int faster_gui_remote_cmd (char* ip, unsigned short port, char* cmd) {
   struct sockaddr_in sa;
   int    SocketFD;
   char   rep [256];
   int    n_rep;
   SocketFD = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (SocketFD == -1) {
      perror ("cannot create socket");
      return (EXIT_FAILURE);
   }
   memset (&sa, 0, sizeof sa);
   sa.sin_family = AF_INET;
   sa.sin_port   = htons (port);
   inet_pton (AF_INET, ip, &sa.sin_addr);
   if (connect (SocketFD, (struct sockaddr *)&sa, sizeof sa) == -1) {
      perror ("connect failed");
      close  (SocketFD);
      return (EXIT_FAILURE);
   }
   if (send (SocketFD, cmd, strlen (cmd), 0) < 0) {
      perror ("send error\n");
      return EXIT_FAILURE;
   }   
   if ((n_rep = recv (SocketFD, rep, 255, 0)) < 0) {
      perror ("recv error\n");
      return EXIT_FAILURE;
   } else {
      rep [n_rep] = '\0';
      printf ("%s\n", rep);
   }
   shutdown (SocketFD, SHUT_RDWR);
   close    (SocketFD);
   return EXIT_SUCCESS;
}

int faster_gui_start (char* ip, unsigned short port, char* filename) {
   char cmd [256];
   if (strcmp (filename, "")) sprintf (cmd, "%s %s", "START", filename);
   else                       sprintf (cmd, "%s",    "START");
   return faster_gui_remote_cmd (ip, port, cmd);
}

int faster_gui_stop (char* ip, unsigned short port) {
   return faster_gui_remote_cmd (ip, port, "STOP");
}
