//
// RN_P_Beispielclient.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
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
  #define NO_FLAGS 0
#endif

#define PROTOPORT		5000		// Default Portnummer
#define AF_INET_LEN		4			// Länge IPv4-Adresse in Byte

int main(int argc, char* argv[])
{
//--------------------------------------------------------------------------------------
// Program:   RN_P_Beispielclient.cpp
//
// Funktion:  Baut eine TCP-Verbindung zu einem Server auf, gibt dessen Nachricht
//			  auf dem Bildschirm aus und bestätigt die Verbindung.
//
// Aufruf:    RN_P_Beispielclient [<Rechnername>][<Portnummer>]
//            <Rechnername>: Rechnername des Servers
//            <Portnummer>: Portnummer unter welcher der Server seinen 
//							Dienst anbietet		
//
// Bemerkung: Die Eingabe der Parameter ist optional. Falls kein Rechnername
//            spezifiziert ist, verwendet der Client "localhost" (127.0.0.1);
//			  falls kein Port spezifiziert ist, wird der durch PROTOPORT definierte
//            Default-Wert verwendet!
//
// Dr. W. Schmitt, 10.12.09
//--------------------------------------------------------------------------------------
	const char *Datum = "26.11.2010";
	char localhost[] = "localhost";		// Default Hostname 
    struct hostent			*ptrh;				// Speichert den Namen des Clientrechners
    struct protoent		*ptrp;				// Speichert den Namen des Transp.Protok.
    struct sockaddr_in		sad;				// Speichert die Serveradresse
    SOCKET			sd;					// Socket Descriptor
	unsigned short	port;				// Portnummer des Servers
    char			*host;				// Host Name static_cast<void*>
    int				n;					// Anzahl empfangener Bytes
    char			RecvBuf[1000];		// Puffer für Empfangsdaten
    char			SendBuf[1000];		// Puffer für Sendedaten
	const char *SendeText = "TCP-Verbindungsaufbau erfolgreich!\n";

#ifdef WIN32
	int				Fehlerkode;			// Hilfsvariable zum Speichern eines Fehlerkodes
	WSADATA			wsaData;			// Speichert Winsock-Initialisierungsinfo
    WORD wVersionRequested;				// Speichert die gewünschte Version der Winsock-
										// DLL
	wVersionRequested = MAKEWORD(2, 2);
// Initialisierung der Winsock-DLL	
	Fehlerkode = WSAStartup(wVersionRequested, &wsaData);
	if (Fehlerkode != NO_ERROR){
		fprintf_s(stdout,"Initialisierung der Winsock-DLL fehlgeschlagen: %i\n",
					Fehlerkode);}
#endif
	
//  Bildschirmausgabe einer Begrüßungsnachricht        
	fprintf(stdout, "TCP-Beispielclient: Version %s\n\n", Datum);

    memset(&sad, 0, sizeof(sad));       // Initialisiere sad mit 0
    sad.sin_family = AF_INET;           // Setze "Internetadressierung"

// Auswerten der Kommandozeilenparameter      
	if (argc > 1) {						// Falls <Rechnername> vorhanden
		host = argv[1];}				// dann in host speichern
	else {
		host = localhost;}				// sonst verwende "localhost"
		
	ptrh = gethostbyname(host);	        // <Rechnername> --> IP-Adresse
        
	if (ptrh == NULL) {
        fprintf(stdout,"ungültige Rechneradresse: %s\n", host);
#ifdef WIN32
		WSACleanup();
#endif
		exit(EXIT_FAILURE);}
		
    if (argc > 2) {                     // Falls <Portnummer> vorhanden
		port = atoi(argv[2]);}			// dann konvertiere nach int
    else {
		port = PROTOPORT;}				// sonst verwende Default-Wert    
        
    if (port > 0)            
		sad.sin_port = htons(port);		// Host --> Netzwerk-Byteordnung
	else {								// Drucke Fehlermeldung; Programmende
		fprintf(stdout,"Ungültige Portnummer %s\n", argv[2]);
#ifdef WIN32
		WSACleanup();
#endif
		exit(EXIT_FAILURE);}
         
#ifdef WIN32
	if (memcpy_s(&sad.sin_addr, AF_INET_LEN, ptrh->h_addr, ptrh->h_length)){
#else
	if (memcpy (&sad.sin_addr, ptrh->h_addr, AF_INET_LEN) == NULL) {
#endif
		fprintf(stdout,"Fehler beim Kopieren der Rechneradresse\n");
#ifdef WIN32
		WSACleanup();
#endif
		exit(EXIT_FAILURE);}
		
    if ((ptrp = getprotobyname("tcp")) == NULL) { // "tcp" --> Protokollcode
        fprintf(stdout, "Abbildung von \"tcp\" auf Protokollnummer nicht möglich!\n");
#ifdef WIN32
		WSACleanup();
#endif
		exit(EXIT_FAILURE);}
        
// Erzeuge Socket
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto); 
    if (sd == INVALID_SOCKET) {
#ifdef WIN32
		fprintf_s(stdout, "Fehler bei Generierung von Socket: %ld\n", WSAGetLastError());
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
		exit(EXIT_FAILURE);}

// Aktiver Verbindungsaufbau zum Server
	if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) == SOCKET_ERROR) {
#ifdef WIN32
		fprintf_s(stdout,"Fehler beim TCP-Verbindungsaufbau: %ld\n", WSAGetLastError());
		WSACleanup();
#else
		if(errno == EACCES) fprintf(stderr,
			"For Unix domain sockets, which are identified by pathname: Write permission is denied on the socket file, or search permission is denied for one of the directories in the path prefix. (See also path_resolution(2).)\n");
		if(errno == EACCES || errno == EPERM) fprintf(stderr,
			"The user tried to connect to a broadcast address without having the socket broadcast flag enabled or the connection request failed because of a local firewall rule.\n");
		if(errno == EADDRINUSE) fprintf(stderr,
			"Local address is already in use.\n");
		if(errno == EAFNOSUPPORT) fprintf(stderr,
			"The passed address didn't have the correct address family in its sa_family field.\n");
		if(errno == EAGAIN) fprintf(stderr,
			"No more free local ports or insufficient entries in the routing cache. For PF_INET see the net.ipv4.ip_local_port_range sysctl in ip(7) on how to increase the number of local ports.\n");
		if(errno == EALREADY) fprintf(stderr,
			"The socket is non-blocking and a previous connection attempt has not yet been completed.\n");
		if(errno == EBADF) fprintf(stderr,
			"The file descriptor is not a valid index in the descriptor table.\n");
		if(errno == ECONNREFUSED) fprintf(stderr,
			"No one listening on the remote address.\n");
		if(errno == EFAULT) fprintf(stderr,
			"The socket structure address is outside the user's address space.\n");
		if(errno == EINPROGRESS) fprintf(stderr,
			"The socket is non-blocking and the connection cannot be completed immediately. It is possible to select(2) or poll(2) for completion by selecting the socket for writing. After select(2) indicates writability, use getsockopt(2) to read the SO_ERROR option at level SOL_SOCKET to determine whether connect() completed successfully (SO_ERROR is zero) or unsuccessfully (SO_ERROR is one of the usual error codes listed here, explaining the reason for the failure).\n");
		if(errno == EINTR) fprintf(stderr,
			"The system call was interrupted by a signal that was caught.\n");
		if(errno == EISCONN) fprintf(stderr,
			"The socket is already connected.\n");
		if(errno == ENETUNREACH) fprintf(stderr,
			"Network is unreachable.\n");
		if(errno == ENOTSOCK) fprintf(stderr,
			"The file descriptor is not associated with a socket.\n");
		if(errno == ETIMEDOUT) fprintf(stderr,
			"Timeout while attempting connection. The server may be too busy to accept new connections. Note that for IP sockets the timeout may be very long when syncookies are enabled on the server.\n");
#endif
		exit(EXIT_FAILURE);}

// Empfangene Daten lesen und auf dem Bildschirm schreiben
	// Daten aus TCP-Empfangspuffer lesen und in RecvBuf schreiben    
	n = recv(sd, RecvBuf, sizeof(RecvBuf), NO_FLAGS); 
	RecvBuf[n] = '\0';					// String mit '\0' abschließen
	if (fprintf(stdout, "%s", RecvBuf) < 0){
		fprintf(stderr, "Schreib-/Lesefehler!\n");}

// Eine Nachricht an den Server senden
	int i = 0;
	while (SendeText[i] != '\0'){
		SendBuf[i]=SendeText[i];
		i++;}
	SendBuf[i]= '\0';
	if (send(sd, SendBuf, strlen(SendBuf), 0) <0){
		fprintf(stdout, "Fehler beim Senden!\n");}

#ifdef WIN32
// Socket schließen und von der Winsock-DLL reservierten Ressourcen freigeben 
	closesocket(sd);
	WSACleanup();
// Programm mit Tastendruck beenden
	fprintf_s(stdout,"Beenden mit Tastendruck!\n" );
	while(!_kbhit()); 
#else
	close(sd);
#endif
	exit(EXIT_SUCCESS);
} // main


