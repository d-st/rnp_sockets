// RN_P_Beispielserver.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#ifdef WIN32
  #incude <winsock.h>
  #include "stdafx.h"
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <errno.h>
  #include <netdb.h>
  #include <string.h>
  #define SOCKET int
  #define INVALID_SOCKET (-1)
  #define SOCKET_ERROR (-1)
#endif

#define PROTOPORT		5000	// Default Portnummer
#define QLEN			6	// Länge der Backlog-Warteschlange
#define AF_INET_LEN		4	// Länge der IPv4-Adresse in Byte
#define RECVBUF_SIZE	1000	// Größe des Empfangspuffers
#define SENDBUF_SIZE	1000	// Größe des Sendepuffers

int main(int argc, char *argv[])
{
//---------------------------------------------------------------------------------
// Program:   RN_P_Beispielserver.cpp
//
// Funktion:  Erzeugt einen Socket und führt die folgenden Aktionen zyklisch aus:
//            (1) Wartet auf eingehende Verbindungswünsche von Clients
//            (2) Führe passiven TCP-Verbindungsaufbau durch
//                (3) Sende eine Nachricht an den Client
//            (4) Schließe die TCP-Verbindung
//            (5) Gehe zurück zu Schritt (1)
//                         
// Aufruf:    RN_P_Beispielserver [<Portnummer>]
//            <Portnummer>: Portnummer unter welcher der Server seinen 
//                                                      Dienst anbietet         
//
// Bemerkung: Die Eingabe der Portnummer ist optional. Falls kein Port spezifiziert
//                        ist, wird der durch PROTOPORT definierte Default-Wert verwendet!    
//
// Dr. W. Schmitt, 10.12.09
//---------------------------------------------------------------------------------

  const char *Datum = "26.11.2010";
  int Fehlerkode;		// Hilfsvariable zum Speichern eines Fehlerkodes
  struct protoent *ptrp;		// Speichert den Namen des Transp.Protok.--> winsock.h
  struct sockaddr_in sad;		// Speichert die Serveradresse --> winsock.h
  struct sockaddr_in cad;		// Speichert die Clientadresse --> winsock.h
  SOCKET sd_accept,		// Socketdeskriptor --> Annahme ank. Verbindungen
         sd_client;			// Socketdeskriptor --> Aktive Clientverbindungen
  unsigned short port;		// Portnummer des Servers
#ifdef WIN32
  int struct_size;		// Größe einer Datenstruktur
#else
  socklen_t struct_size;		// Größe einer Datenstruktur
#endif
  int string_len;		// Länge einer Zeichenkette
  int n;			// Anzahl empfangener Bytes
  char RecvBuf[RECVBUF_SIZE];	// Puffer für Empfangsdaten
  char SendBuf[SENDBUF_SIZE];	// Puffer für Sendedaten
  int visits = 0;		// Zählt die Anzahl erfolgreicher Clientverbindungen


#ifdef WIN32
  WSADATA wsaData;		// Speichert Winsock-Initialisierungsinfo
  WORD wVersionRequested;	// Speichert die gewünschte Version der Winsock-DLL

  wVersionRequested = MAKEWORD(2, 2);
// Initialisierung der Winsock-DLL      
  Fehlerkode = WSAStartup(wVersionRequested, &wsaData);
  if (Fehlerkode != NO_ERROR) {
    fprintf_s(stdout,
	      "Initialisierung der Winsock-DLL fehlgeschlagen: %i\n",
	      Fehlerkode);
  }
#endif

  //  Bildschirmausgabe einer Begrüßungsnachricht         
  fprintf(stdout, "TCP-Beispielserver: Version %s\n\n", Datum);

  memset(&sad, 0, sizeof(sad));	// Initialisiere sad mit 0
  sad.sin_family = AF_INET;	// Setze "Internetadressierung"
  sad.sin_addr.s_addr = INADDR_ANY;	//

// Auswerten der Kommandozeilenparameter      

  if (argc > 1) {		// Falls <Portnummer> vorhanden
    port = atoi(argv[1]);
  }				// dann konvertiere nach int
  else {
    port = PROTOPORT;
  }				// sonst verwende Default-Wert 

  if (port > 0)
    sad.sin_port = htons(port);	// Host --> Netzwerk-Byteordnung
  else {			// Drucke Fehlermeldung; Programmende
    fprintf(stdout, "Ungültige Portnummer %s\n", argv[1]);
#ifdef WIN32
    WSACleanup();
#endif
    exit(EXIT_FAILURE);
  }

  if ((ptrp = getprotobyname("tcp")) == NULL) {	// "tcp" --> Protokollcode
    fprintf(stdout,
	    "Abbildung von \"tcp\" auf Protokollnummer nicht möglich!");
#ifdef WIN32
    WSACleanup();
#endif
    exit(EXIT_FAILURE);
  }
// Erzeuge Socket
  sd_accept = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
  if (sd_accept == INVALID_SOCKET) {
#ifdef WIN32
    fprintf_s(stdout, "Fehler bei Generierung von Socket: %d\n",
	      WSAGetLastError());
    WSACleanup();
#else
	if(errno == EACCES) fprintf(stderr,
		"Permission to create a socket of the specified type and/or protocol is denied.\n");
	if(errno == EAFNOSUPPORT) fprintf(stderr,
		"The implementation does not support the specified address family.\n");
	if(errno == EINVAL) fprintf(stderr,
		"Unknown protocol, or protocol family not available.\n");
	if(errno == EMFILE) fprintf(stderr,
		"Process file table overflow.\n");
	if(errno == ENFILE) fprintf(stderr,
		"The system limit on the total number of open files has been reached.\n");
	if(errno == ENOBUFS || errno == ENOMEM) fprintf(stderr,
		"Insufficient memory is available. The socket cannot be created until sufficient resources are freed.\n");
	if(errno == EPROTONOSUPPORT) fprintf(stderr,
		"The protocol type or the specified protocol is not supported within this domain.\n");
#endif
    exit(EXIT_FAILURE);
  }
// Initialisiere Socket mit (Rechneradress, Portnummer) des Servers
  if (bind(sd_accept, (struct sockaddr *) &sad, sizeof(sad)) ==
      SOCKET_ERROR) {
#ifdef WIN32
    fprintf_s(stdout, "bind-Fehler: %d\n", WSAGetLastError());
    WSACleanup();
#else
	if(errno == EACCES) fprintf(stderr,
		"The address is protected, and the user is not the superuser.\n");
	if(errno == EADDRINUSE) fprintf(stderr,
		"The given address is already in use.\n");
	if(errno == EBADF) fprintf(stderr,
		"sockfd is not a valid descriptor.\n");
	if(errno == EINVAL) fprintf(stderr,
		"The socket is already bound to an address.\n");
	if(errno == ENOTSOCK) fprintf(stderr,
		"sockfd is a descriptor for a file, not a socket.\n");
#endif
    exit(EXIT_FAILURE);
  }
// Lege Länge der Backlog-Warteschlange fest
  if (listen(sd_accept, QLEN) == SOCKET_ERROR) {
#ifdef WIN32
    fprintf_s(stdout, "listen-Fehler: %d\n", WSAGetLastError());
    WSACleanup();
#else
	if(errno == EADDRINUSE) fprintf(stderr,
		"Another socket is already listening on the same port.\n");
	if(errno == EBADF) fprintf(stderr,
		"The argument sockfd is not a valid descriptor.\n");
	if(errno == ENOTSOCK) fprintf(stderr,
		"The argument sockfd is not a socket.\n");
	if(errno == EOPNOTSUPP) fprintf(stderr,
		"The socket is not of a type that supports the listen() operation.\n");
#endif
    exit(EXIT_FAILURE);
  }
// Hauptschleife: Annahme und Bearbeitung von Verbindungswünschen
  while (1) {
    struct_size = sizeof(cad);
    if ((sd_client = accept(sd_accept, (struct sockaddr *) &cad,
		&struct_size)) == INVALID_SOCKET) {
#ifdef WIN32
      fprintf_s(stdout, "accept-Fehler: %d\n", WSAGetLastError());
	  WSACleanup();
#else
		if(errno == EAGAIN || errno == EWOULDBLOCK) fprintf(stderr,
			"The socket is marked non-blocking and no connections are present to be accepted.\n");
		if(errno == EBADF) fprintf(stderr,
			"The descriptor is invalid.\n");
		if(errno == ECONNABORTED) fprintf(stderr,
			"A connection has been aborted.\n");
		if(errno == EINTR) fprintf(stderr,
			"The system call was interrupted by a signal that was caught before a valid connection arrived.\n");
		if(errno == EINVAL) fprintf(stderr,
			"Socket is not listening for connections, or addrlen is invalid (e.g., is negative).\n");
		if(errno == EMFILE) fprintf(stderr,
			"The per-process limit of open file descriptors has been reached.\n");
		if(errno == ENFILE) fprintf(stderr,
			"The system limit on the total number of open files has been reached.\n");
		if(errno == ENOTSOCK) fprintf(stderr,
			"The descriptor references a file, not a socket.\n");
		if(errno == EOPNOTSUPP) fprintf(stderr,
			"The referenced socket is not of type SOCK_STREAM.\n");
		if(errno == EFAULT) fprintf(stderr,
			"The addr argument is not in a writable part of the user address space.\n");
		if(errno == ENOBUFS, ENOMEM) fprintf(stderr,
			"Not enough free memory. This often means that the memory allocation is limited by the socket buffer limits, not by the system memory.\n");
		if(errno == EPROTO) fprintf(stderr,
			"Protocol error.\n");
		// Linux accept() only
		if(errno == EPERM) fprintf(stderr,
			"Firewall rules forbid connection.\n");
#endif
      exit(EXIT_FAILURE);
    }

    visits++;
    sprintf(SendBuf, "Dieser Server wurde %d mal besucht!\n", visits);
    string_len = strlen(SendBuf);
    send(sd_client, SendBuf, string_len, 0);

// Empfangene Daten lesen, in RecvBuf schreiben ...
    n = recv(sd_client, RecvBuf, RECVBUF_SIZE, 0);
    RecvBuf[n] = '\0';		// String mit '\0' abschließen
    if (fprintf(stdout, RecvBuf, n) < 0) {	// und auf dem Bildschirm ausgeben
      fprintf(stdout, "Fehler beim Lesen/Schreiben!\n");
    }

#ifdef WIN32
    closesocket(sd_client);
#else
    close(sd_client);
#endif
  } // Hauptschleife

#ifdef WIN32
    WSACleanup();
#endif
  exit(EXIT_SUCCESS);
}				// main
