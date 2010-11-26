// RN_P_Beispielclient.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//
#include "stdafx.h"

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
	const char *Datum = "09.01.09";
	int				Fehlerkode;			// Hilfsvariable zum Speichern eines Fehlerkodes
	char localhost[] = "localhost";		// Default Hostname 
    hostent			*ptrh;				// Speichert den Namen des Clientrechners
    protoent		*ptrp;				// Speichert den Namen des Transp.Protok.
    sockaddr_in		sad;				// Speichert die Serveradresse
    SOCKET			sd;					// Socket Descriptor
	unsigned short	port;				// Portnummer des Servers
    char			*host;				// Host Name static_cast<void*>
    int				n;					// Anzahl empfangener Bytes
    char			RecvBuf[1000];		// Puffer für Empfangsdaten
    char			SendBuf[1000];		// Puffer für Sendedaten
	char *SendeText = "TCP-Verbindungsaufbau erfolgreich! \n";
	WSADATA			wsaData;			// Speichert Winsock-Initialisierungsinfo
    WORD wVersionRequested;				// Speichert die gewünschte Version der Winsock-
										// DLL
	wVersionRequested = MAKEWORD(2, 2);
// Initialisierung der Winsock-DLL	
	Fehlerkode = WSAStartup(wVersionRequested, &wsaData);
	if (Fehlerkode != NO_ERROR){
		fprintf_s(stdout,"Initialisierung der Winsock-DLL fehlgeschlagen: %i\n",
					Fehlerkode);}
	
//  Bildschirmausgabe einer Begrüßungsnachricht        
	fprintf_s(stdout, "TCP-Beispielclient: Version %s\n\n", Datum);

    memset(&sad, 0, sizeof(sad));       // Initialisiere sad mit 0
    sad.sin_family = AF_INET;           // Setze "Internetadressierung"

// Auswerten der Kommandozeilenparameter      
	if (argc > 1) {						// Falls <Rechnername> vorhanden
		host = argv[1];}				// dann in host speichern
	else {
		host = localhost;}				// sonst verwende "localhost"
		
	ptrh = gethostbyname(host);	        // <Rechnername> --> IP-Adresse
        
	if (ptrh == NULL) {
        fprintf_s(stdout,"ungültige Rechneradresse: %s\n", host);
		WSACleanup();
		exit(EXIT_FAILURE);}
		
    if (argc > 2) {                     // Falls <Portnummer> vorhanden
		port = atoi(argv[2]);}			// dann konvertiere nach int
    else {
		port = PROTOPORT;}				// sonst verwende Default-Wert    
        
    if (port > 0)            
		sad.sin_port = htons(port);		// Host --> Netzwerk-Byteordnung
	else {								// Drucke Fehlermeldung; Programmende
		fprintf_s(stdout,"Ungültige Portnummer %s\n",argv[2]);
		WSACleanup();
		exit(EXIT_FAILURE);}
         
	if (memcpy_s(&sad.sin_addr, AF_INET_LEN, ptrh->h_addr, ptrh->h_length)){
		fprintf_s(stdout,"Fehler beim Kopieren der Rechneradresse");
		WSACleanup();
		exit(EXIT_FAILURE);}
		
    if ((ptrp = getprotobyname("tcp")) == NULL) { // "tcp" --> Protokollcode
        fprintf_s(stdout, "Abbildung von \"tcp\" auf Protokollnummer nicht möglich!");
		WSACleanup();
		exit(EXIT_FAILURE);}
        
// Erzeuge Socket
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto); 
    if (sd == INVALID_SOCKET) {
		fprintf_s(stdout, "Fehler bei Generierung von Socket: %ld\n", WSAGetLastError());
		WSACleanup();
		exit(EXIT_FAILURE);}

// Aktiver Verbindungsaufbau zum Server
	if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) == SOCKET_ERROR) {
		fprintf_s(stdout,"Fehler beim TCP-Verbindungsaufbau: %ld\n", WSAGetLastError());
		WSACleanup();
		exit(EXIT_FAILURE);}

// Empfangene Daten lesen und auf dem Bildschirm schreiben
	// Daten aus TCP-Empfangspuffer lesen und in RecvBuf schreiben    
	n = recv(sd, RecvBuf, static_cast<int>(strlen(RecvBuf)), 0); 
	RecvBuf[n] = '\0';					// String mit '\0' abschließen
	if (fprintf_s(stdout, RecvBuf, n) < 0){
		fprintf_s(stdout, "Schreib-/Lesefehler!\n");}

// Eine Nachricht an den Server senden
	int i = 0;
	while (SendeText[i] != '\0'){
		SendBuf[i]=SendeText[i];
		i++;}
	SendBuf[i]= '\0';
	if (send(sd, SendBuf, static_cast<int>(strlen(SendBuf)), 0) <0){
		fprintf_s(stdout, "Fehler beim Senden!\n");}

// Socket schließen und von der Winsock-DLL reservierten Ressourcen freigeben 
	closesocket(sd);
	WSACleanup();

// Programm mit Tastendruck beenden
	fprintf_s(stdout,"Beenden mit Tastendruck! \n" );
	while(!_kbhit()); 

	exit(EXIT_SUCCESS);
} // main


