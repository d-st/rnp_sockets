// RN_P_Beispielserver.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//
#include "stdafx.h"

#define PROTOPORT		5000    // Default Portnummer
#define QLEN			6       // Länge der Backlog-Warteschlange
#define AF_INET_LEN		4		// Länge der IPv4-Adresse in Byte
#define RECVBUF_SIZE	1000	// Größe des Empfangspuffers
#define SENDBUF_SIZE	1000	// Größe des Sendepuffers

int main(int argc, char* argv[])
{
//---------------------------------------------------------------------------------
// Program:   RN_P_Beispielserver.cpp
//
// Funktion:  Erzeugt einen Socket und führt die folgenden Aktionen zyklisch aus:
//            (1) Wartet auf eingehende Verbindungswünsche von Clients
//            (2) Führe passiven TCP-Verbindungsaufbau durch
//	          (3) Sende eine Nachricht an den Client
//            (4) Schließe die TCP-Verbindung
//            (5) Gehe zurück zu Schritt (1)
//			   
// Aufruf:    RN_P_Beispielserver [<Portnummer>]
//            <Portnummer>: Portnummer unter welcher der Server seinen 
//							Dienst anbietet		
//
// Bemerkung: Die Eingabe der Portnummer ist optional. Falls kein Port spezifiziert
//			  ist, wird der durch PROTOPORT definierte Default-Wert verwendet!    
//
// Dr. W. Schmitt, 10.12.09
//---------------------------------------------------------------------------------

	const char *Datum = "09.01.09";
    int		Fehlerkode;		// Hilfsvariable zum Speichern eines Fehlerkodes
	protoent *ptrp;			// Speichert den Namen des Transp.Protok.--> winsock.h
	sockaddr_in sad;		// Speichert die Serveradresse --> winsock.h
    sockaddr_in cad;		// Speichert die Clientadresse --> winsock.h
    SOCKET  sd_accept,		// Socketdeskriptor --> Annahme ank. Verbindungen
			sd_client;      // Socketdeskriptor --> Aktive Clientverbindungen
    unsigned short	port;	// Portnummer des Servers
    int     struct_size;    // Größe einer Datenstruktur
    int		string_len;		// Länge einer Zeichenkette
	int		n;				// Anzahl empfangener Bytes
	char	RecvBuf[RECVBUF_SIZE];	// Puffer für Empfangsdaten
	char    SendBuf[SENDBUF_SIZE];  // Puffer für Sendedaten
	int     visits = 0;		// Zählt die Anzahl erfolgreicher Clientverbindungen
	WSADATA	wsaData;		// Speichert Winsock-Initialisierungsinfo
    WORD wVersionRequested;	// Speichert die gewünschte Version der Winsock-DLL

	wVersionRequested = MAKEWORD(2, 2);
// Initialisierung der Winsock-DLL	
	Fehlerkode = WSAStartup(wVersionRequested, &wsaData);
	if (Fehlerkode != NO_ERROR){
		fprintf_s(stdout,"Initialisierung der Winsock-DLL fehlgeschlagen: %i\n",
					Fehlerkode);
	} 

	//  Bildschirmausgabe einer Begrüßungsnachricht     	
	fprintf_s(stdout, "TCP-Beispielserver: Version %s\n\n", Datum);

	memset(&sad, 0, sizeof(sad));       // Initialisiere sad mit 0
	sad.sin_family = AF_INET;           // Setze "Internetadressierung"
    sad.sin_addr.s_addr = INADDR_ANY;   //

// Auswerten der Kommandozeilenparameter      

	if (argc > 1) {					// Falls <Portnummer> vorhanden
		port = atoi(argv[1]);}		// dann konvertiere nach int
    else {
		port = PROTOPORT;}			// sonst verwende Default-Wert 
        
    if (port > 0)
        sad.sin_port = htons(port); // Host --> Netzwerk-Byteordnung
    else {                          // Drucke Fehlermeldung; Programmende
         fprintf(stdout,"Ungültige Portnummer %s\n",argv[1]);
		 WSACleanup();
		 exit(EXIT_FAILURE);}

    if ((ptrp = getprotobyname("tcp")) == NULL){ // "tcp" --> Protokollcode
        fprintf(stdout, "Abbildung von \"tcp\" auf Protokollnummer nicht möglich!");
		WSACleanup();
		exit(EXIT_FAILURE);}

// Erzeuge Socket
    sd_accept = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sd_accept == INVALID_SOCKET) {
		fprintf_s(stdout, "Fehler bei Generierung von Socket: %ld\n", WSAGetLastError());
		WSACleanup();
		exit(EXIT_FAILURE);}

// Initialisiere Socket mit (Rechneradress, Portnummer) des Servers
    if (bind(sd_accept, (struct sockaddr*)&sad, sizeof(sad)) == SOCKET_ERROR) {
        fprintf(stdout,"bind-Fehler: %ld\n", WSAGetLastError());
		WSACleanup();
		exit(EXIT_FAILURE);}

// Lege Länge der Backlog-Warteschlange fest
	if (listen(sd_accept, QLEN) == SOCKET_ERROR) {
        fprintf(stdout, "listen-Fehler: %ld\n", WSAGetLastError());
		WSACleanup();
		exit(EXIT_FAILURE);}

// Hauptschleife: Annahme und Bearbeitung von Verbindungswünschen
	while (1) {
		struct_size = sizeof(cad);
		if ((sd_client=accept(sd_accept, (struct sockaddr*)&cad, &struct_size)) == INVALID_SOCKET) {
          fprintf(stdout, "accept-Fehler: %ld\n", WSAGetLastError());
		  WSACleanup();
		  exit(EXIT_FAILURE);}

		visits++;
		sprintf_s(SendBuf,"Dieser Server wurde %d mal besucht!\n", visits);
		string_len = static_cast<int>(strlen(SendBuf));
		send(sd_client, SendBuf, string_len, 0);

// Empfangene Daten lesen, in RecvBuf schreiben ...
		n = recv(sd_client, RecvBuf, RECVBUF_SIZE, 0);
		RecvBuf[n] = '\0';							// String mit '\0' abschließen
		if (fprintf_s(stdout, RecvBuf, n) < 0){		// und auf dem Bildschirm ausgeben
			fprintf_s(stdout, "Fehler beim Lesen/Schreiben!\n");}

		closesocket(sd_client);
    } // Hauptschleife

	WSACleanup();	
	exit(EXIT_SUCCESS);
} // main

