
#include "serror.h"

const char* RNPGetLastError() {
#ifdef WIN32
	return WSAGetLastError();
#else
	switch(errno) {
		case EACCES: 
			return "Permission to create a socket of the specified type and/or protocol is denied.";
		break;
		case EAFNOSUPPORT:
			return "The implementation does not support the specified address family.";
		break;
		case EINVAL:
			return "Unknown protocol, or protocol family not available.";
		break;
		case EMFILE:
			return "Process file table overflow.";
		break;
		case ENFILE:
			return "The system limit on the total number of open files has been reached.";
		break;
		case ENOBUFS: case ENOMEM: /* two conditions are intended */
			return "Insufficient memory is available. The socket cannot be created until sufficient resources are freed.";
		break;
		case EPROTONOSUPPORT:
			return "The protocol type or the specified protocol is not supported within this domain.";
		break;
	}
	return "unknown error";
#endif
}

