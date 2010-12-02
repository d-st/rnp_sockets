#include "serror.h"
#include <stdio.h>

#ifdef WIN32
  #include <WinSock2.h>
#endif


void RNP_Cleanup() {
#ifdef WIN32
	WSACleanup();
#endif
}

void RNP_Error(const int RNP_E_type, const char * msg) {
#ifdef WIN32
    fprintf_s(stdout, "%s: %d\n", msg, WSAGetLastError());
#else
    fprintf(stderr, "%s:\n", msg);
	// socket() errors
	if(RNP_E_type == RNP_E_SOCK) switch(errno) {
		case EACCES: 
			fprintf(stderr, "Permission to create a socket of the specified type and/or protocol is denied.\n");
		break;
		case EAFNOSUPPORT:
			fprintf(stderr, "The implementation does not support the specified address family.\n");
		break;
		case EINVAL:
			fprintf(stderr, "Unknown protocol, or protocol family not available.\n");
		break;
		case EMFILE:
			fprintf(stderr, "Process file table overflow.\n");
		break;
		case ENFILE:
			fprintf(stderr, "The system limit on the total number of open files has been reached.\n");
		break;
		case ENOBUFS: case ENOMEM: // two conditions are intended
			fprintf(stderr, "Insufficient memory is available. The socket cannot be created until sufficient resources are freed.\n");
		break;
		case EPROTONOSUPPORT:
			fprintf(stderr, "The protocol type or the specified protocol is not supported within this domain.\n");
		break;
		default: fprintf(stderr, "unknown error\n"); break;
	} else
	// bind() errors
	if(RNP_E_type == RNP_E_BIND) switch(errno) {
		case EACCES: 
			fprintf(stderr,	"The address is protected, and the user is not the superuser.\n");
		break;
		case EADDRINUSE: 
			fprintf(stderr,	"The given address is already in use.\n");
		break;
		case EBADF: 
			fprintf(stderr,	"sockfd is not a valid descriptor.\n");
		break;
		case EINVAL: 
			fprintf(stderr,	"The socket is already bound to an address.\n");
		break;
		case ENOTSOCK: 
			fprintf(stderr,	"sockfd is a descriptor for a file, not a socket.\n");
		break;
		default: fprintf(stderr, "unknown error\n"); break;
	} else
	// listen() errors
	if(RNP_E_type == RNP_E_LSTN) switch(errno) {
		case EADDRINUSE:
			fprintf(stderr,	"Another socket is already listening on the same port.\n");
		break;
		case EBADF:
			fprintf(stderr,	"The argument sockfd is not a valid descriptor.\n");
		break;
		case ENOTSOCK:
			fprintf(stderr,	"The argument sockfd is not a socket.\n");
		break;
		case EOPNOTSUPP:
			fprintf(stderr,	"The socket is not of a type that supports the listen() operation.\n");
		break;
		default: fprintf(stderr, "unknown error\n"); break;
	} else
	// accept() errors
	if(RNP_E_type == RNP_E_ACPT) switch(errno) {
		case EAGAIN: // case EWOULDBLOCK: // two conditions are intended, but defined equal
			fprintf(stderr,	"The socket is marked non-blocking and no connections are present to be accepted.\n");
		break;
		case EBADF:
			fprintf(stderr,	"The descriptor is invalid.\n");
		break;
		case ECONNABORTED:
			fprintf(stderr, "A connection has been aborted.\n");
		break;
		case EINTR:
			fprintf(stderr,	"The system call was interrupted by a signal that was caught before a valid connection arrived.\n");
		break;
		case EINVAL:
			fprintf(stderr, "Socket is not listening for connections, or addrlen is invalid (e.g., is negative).\n");
		break;
		case EMFILE:
			fprintf(stderr, "The per-process limit of open file descriptors has been reached.\n");
		break;
		case ENFILE:
			fprintf(stderr,	"The system limit on the total number of open files has been reached.\n");
		break;
		case ENOTSOCK:
			fprintf(stderr,	"The descriptor references a file, not a socket.\n");
		break;
		case EOPNOTSUPP:
			fprintf(stderr,	"The referenced socket is not of type SOCK_STREAM.\n");
		break;
		case EFAULT:
			fprintf(stderr,	"The addr argument is not in a writable part of the user address space.\n");
		break;
		case ENOBUFS: case ENOMEM: // two conditions are intended
			fprintf(stderr,	"Not enough free memory. This often means that the memory allocation is limited by the socket buffer limits, not by the system memory.\n");
		break;
		case EPROTO:
			fprintf(stderr,	"Protocol error.\n");
		break;
		case EPERM: // Linux-style accept() only
			fprintf(stderr,	"Firewall rules forbid connection.\n");
		break;
		default: fprintf(stderr, "unknown error\n"); break;
	} else
	// accept() errors
	if(RNP_E_type == RNP_E_CONN) switch(errno) {
		case EACCES:
			fprintf(stderr,	"For Unix domain sockets, which are identified by pathname: Write permission is denied on the socket file, or search permission is denied for one of the directories in the path prefix. (See also path_resolution(2).)\n");
			// two conditions are intended, so no break here. intended!
		case EPERM: 
			fprintf(stderr,	"The user tried to connect to a broadcast address without having the socket broadcast flag enabled or the connection request failed because of a local firewall rule.\n");
		break;
		case EADDRINUSE:
			fprintf(stderr,	"Local address is already in use.\n");
		break;
		case EAFNOSUPPORT:
			fprintf(stderr,	"The passed address didn't have the correct address family in its sa_family field.\n");
		break;
		case EAGAIN:
			fprintf(stderr,	"No more free local ports or insufficient entries in the routing cache. For PF_INET see the net.ipv4.ip_local_port_range sysctl in ip(7) on how to increase the number of local ports.\n");
		break;
		case EALREADY:
			fprintf(stderr,	"The socket is non-blocking and a previous connection attempt has not yet been completed.\n");
		break;
		case EBADF:
			fprintf(stderr,	"The file descriptor is not a valid index in the descriptor table.\n");
		break;
		case ECONNREFUSED:
			fprintf(stderr,	"No one listening on the remote address.\n");
		break;
		case EFAULT:
			fprintf(stderr,	"The socket structure address is outside the user's address space.\n");
		break;
		case EINPROGRESS:
			fprintf(stderr,	"The socket is non-blocking and the connection cannot be completed immediately. It is possible to select(2) or poll(2) for completion by selecting the socket for writing. After select(2) indicates writability, use getsockopt(2) to read the SO_ERROR option at level SOL_SOCKET to determine whether connect() completed successfully (SO_ERROR is zero) or unsuccessfully (SO_ERROR is one of the usual error codes listed here, explaining the reason for the failure).\n");
		break;
		case EINTR:
			fprintf(stderr,	"The system call was interrupted by a signal that was caught.\n");
		break;
		case EISCONN:
			fprintf(stderr,	"The socket is already connected.\n");
		break;
		case ENETUNREACH:
			fprintf(stderr,	"Network is unreachable.\n");
		break;
		case ENOTSOCK:
			fprintf(stderr,	"The file descriptor is not associated with a socket.\n");
		break;
		case ETIMEDOUT:
			fprintf(stderr,	"Timeout while attempting connection. The server may be too busy to accept new connections. Note that for IP sockets the timeout may be very long when syncookies are enabled on the server.\n");
		break;
		default: fprintf(stderr, "unknown error\n"); break;
	} else
	// fehler irgendwo anders, vermutlich beim Aufruf
	fprintf(stderr, "unknown error\n");
#endif
}

