#ifndef _RNP_WRAP_H_
#define _RNP_WRAP_H_

#include <stdio.h>

#ifdef WIN32
  #include "stdafx.h"
  #include <WinSock2.h>
  #include <conio.h>
#else
  #include <unistd.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netdb.h>
  #include <string.h>
  #include <errno.h>
  #include <stdlib.h>
  #define SOCKET int
  #define INVALID_SOCKET (-1)
  #define SOCKET_ERROR (-1)
#endif

#define RNP_E_BIND 1
#define RNP_E_SOCK 2
#define RNP_E_LSTN 3
#define RNP_E_ACPT 4
#define RNP_E_CONN 5

#define NO_FLAGS 0
#define AF_INET_LEN		4			// Länge IPv4-Adresse in Byte


void RNP_Init();
void RNP_Cleanup();
void RNP_Error(const int RNP_E_type, const char * msg);
void RNP_Close(SOCKET sd);
int  RNP_copy_hostaddr_to_sockaddr(struct sockaddr_in * sad, struct hostent * host);

#endif
