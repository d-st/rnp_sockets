#ifndef SERROR_H
#define SERROR_H

#ifndef WIN32
	#include <errno.h>
	#include <stdio.h>
	#include <stdlib.h>
#endif

#define RNP_E_BIND 1
#define RNP_E_SOCK 2
#define RNP_E_LSTN 3
#define RNP_E_ACPT 4
#define RNP_E_CONN 5

void RNP_Cleanup();
void RNP_Error(const int RNP_E_type, const char * msg);

#endif
