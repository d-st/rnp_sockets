// RN_P_Beispielserver.cpp : Definiert den Einstiegspunkt f�r die Konsolenanwendung.
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

#define PROTOPORT		5000	// Default Portnummer
#define QLEN			6	// L�nge der Backlog-Warteschlange
#define AF_INET_LEN		4	// L�nge der IPv4-Adresse in Byte
#define RECVBUF_SIZE	1000	// Gr��e des Empfangspuffers
#define SENDBUF_SIZE	1000	// Gr��e des Sendepuffers

int main(int argc, char *argv[])
{
//---------------------------------------------------------------------------------
// Program:   RN_P_Beispielserver.cpp
//
// Funktion:  Erzeugt einen Socket und f�hrt die folgenden Aktionen zyklisch aus:
//            (1) Wartet auf eingehende Verbindungsw�nsche von Clients
//            (2) F�hre passiven TCP-Verbindungsaufbau durch
//                (3) Sende eine Nachricht an den Client
//            (4) Schlie�e die TCP-Verbindung
//            (5) Gehe zur�ck zu Schritt (1)
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
  struct protoent *ptrp;		// Speichert den Namen des Transp.Protok.--> winsock.h
  struct sockaddr_in sad;		// Speichert die Serveradresse --> winsock.h
  struct sockaddr_in cad;		// Speichert die Clientadresse --> winsock.h
  SOCKET sd_accept,		// Socketdeskriptor --> Annahme ank. Verbindungen
         sd_client;			// Socketdeskriptor --> Aktive Clientverbindungen
  unsigned short port;		// Portnummer des Servers
#ifdef WIN32
  int Fehlerkode;		// Hilfsvariable zum Speichern eines Fehlerkodes
  int struct_size;		// Gr��e einer Datenstruktur
#else
  socklen_t struct_size;		// Gr��e einer Datenstruktur
#endif
  int string_len;		// L�nge einer Zeichenkette
  int n;			// Anzahl empfangener Bytes
  char RecvBuf[RECVBUF_SIZE];	// Puffer f�r Empfangsdaten
  char SendBuf[SENDBUF_SIZE];	// Puffer f�r Sendedaten
  int visits = 0;		// Z�hlt die Anzahl erfolgreicher Clientverbindungen


#ifdef WIN32
  WSADATA wsaData;		// Speichert Winsock-Initialisierungsinfo
  WORD wVersionRequested;	// Speichert die gew�nschte Version der Winsock-DLL

  wVersionRequested = MAKEWORD(2, 2);
// Initialisierung der Winsock-DLL      
  Fehlerkode = WSAStartup(wVersionRequested, &wsaData);
  if (Fehlerkode != NO_ERROR) {
    fprintf_s(stdout,
	      "Initialisierung der Winsock-DLL fehlgeschlagen: %i\n",
	      Fehlerkode);
  }
#endif

  //  Bildschirmausgabe einer Begr��ungsnachricht         
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
    fprintf(stdout, "Ung�ltige Portnummer %s\n", argv[1]);
    RNP_Cleanup();
    exit(EXIT_FAILURE);
  }

  if ((ptrp = getprotobyname("tcp")) == NULL) {	// "tcp" --> Protokollcode
    fprintf(stdout,
	    "Abbildung von \"tcp\" auf Protokollnummer nicht m�glich!");
    RNP_Cleanup();
    exit(EXIT_FAILURE);
  }
// Erzeuge Socket
  sd_accept = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
  if (sd_accept == INVALID_SOCKET) {
	RNP_Error(RNP_E_SOCK, "Fehler bei Generierung von Socket");
    RNP_Cleanup();
    exit(EXIT_FAILURE);
  }
// Initialisiere Socket mit (Rechneradress, Portnummer) des Servers
  if (bind(sd_accept, (struct sockaddr *) &sad, sizeof(sad)) ==
      SOCKET_ERROR) {
	RNP_Error(RNP_E_BIND, "bind-Fehler");
    RNP_Cleanup();
    exit(EXIT_FAILURE);
  }
// Lege L�nge der Backlog-Warteschlange fest
  if (listen(sd_accept, QLEN) == SOCKET_ERROR) {
	RNP_Error(RNP_E_LSTN, "listen-Fehler");
    RNP_Cleanup();
    exit(EXIT_FAILURE);
  }
// Hauptschleife: Annahme und Bearbeitung von Verbindungsw�nschen
  while (1) {
    struct_size = sizeof(cad);
    if ((sd_client = accept(sd_accept, (struct sockaddr *) &cad,
		&struct_size)) == INVALID_SOCKET) {
      RNP_Error(RNP_E_ACPT, "accept-Fehler");
	  RNP_Cleanup();
      exit(EXIT_FAILURE);
    }

    visits++;
    sprintf(SendBuf, "Dieser Server wurde %d mal besucht!\n", visits);
    string_len = strlen(SendBuf);
    send(sd_client, SendBuf, string_len, 0);

// Empfangene Daten lesen, in RecvBuf schreiben ...
    n = recv(sd_client, RecvBuf, RECVBUF_SIZE, 0);
    RecvBuf[n] = '\0';		// String mit '\0' abschlie�en
    if (fprintf(stdout, RecvBuf, n) < 0) {	// und auf dem Bildschirm ausgeben
      fprintf(stdout, "Fehler beim Lesen/Schreiben!\n");
    }

#ifdef WIN32
    closesocket(sd_client);
#else
    close(sd_client);
#endif
  } // Hauptschleife

  RNP_Cleanup();
  exit(EXIT_SUCCESS);
}				// main
