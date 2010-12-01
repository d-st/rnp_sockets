//
// RN_P_Beispielclient.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#ifdef WIN32
  #incude <winsock.h>
  #include "stdafx.h"
#else
  #include <unistd.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netdb.h>
  #include <string.h>
  #define SOCKET int
  #define INVALID_SOCKET (-1)
  #define SOCKET_ERROR (-1)
#endif

#include "serror.h"

#define NO_FLAGS 0

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
		RNP_Cleanup();
		exit(EXIT_FAILURE);}
		
    if (argc > 2) {                     // Falls <Portnummer> vorhanden
		port = atoi(argv[2]);}			// dann konvertiere nach int
    else {
		port = PROTOPORT;}				// sonst verwende Default-Wert    
        
    if (port > 0)            
		sad.sin_port = htons(port);		// Host --> Netzwerk-Byteordnung
	else {								// Drucke Fehlermeldung; Programmende
		fprintf(stdout,"Ungültige Portnummer %s\n", argv[2]);
		RNP_Cleanup();
		exit(EXIT_FAILURE);}
         
#ifdef WIN32
	if (memcpy_s(&sad.sin_addr, AF_INET_LEN, ptrh->h_addr, ptrh->h_length)){
#else
	if (memcpy (&sad.sin_addr, ptrh->h_addr, AF_INET_LEN) == NULL) {
#endif
		fprintf(stdout,"Fehler beim Kopieren der Rechneradresse\n");
		RNP_Cleanup();
		exit(EXIT_FAILURE);}
		
    if ((ptrp = getprotobyname("tcp")) == NULL) { // "tcp" --> Protokollcode
        fprintf(stdout, "Abbildung von \"tcp\" auf Protokollnummer nicht möglich!\n");
		RNP_Cleanup();
		exit(EXIT_FAILURE);}
        
// Erzeuge Socket
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto); 
    if (sd == INVALID_SOCKET) {
		RNP_Error(RNP_E_SOCK, "Fehler bei Generierung des Sockets");
		RNP_Cleanup();
		exit(EXIT_FAILURE);}

// Aktiver Verbindungsaufbau zum Server
	if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) == SOCKET_ERROR) {
#ifdef WIN32
		fprintf_s(stdout,"Fehler beim TCP-Verbindungsaufbau: %ld\n", WSAGetLastError());
		RNP_Cleanup();
#else
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
	RNP_Cleanup();
// Programm mit Tastendruck beenden
	fprintf_s(stdout,"Beenden mit Tastendruck!\n" );
	while(!_kbhit()); 
#else
	close(sd);
#endif
	exit(EXIT_SUCCESS);
} // main


