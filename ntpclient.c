//
// simpler NTP client im Rahmen des RNP
//

#ifdef WIN32
  #include "stdafx.h"
  #include <winsock.h>
  #include <conio.h>
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

#define JAN_1970        2208988800UL /* 1970 - 1900 in seconds */
#define NO_FLAGS 0

#define PROTOPORT		123 		// Default Portnummer
#define AF_INET_LEN		4			// Länge IPv4-Adresse in Byte


/*
 * Transmit packet
 */
typedef unsigned long long tstamp;   /* NTP timestamp format */
typedef unsigned int tdist;     /* NTP short format */
typedef unsigned long ipaddr;   /* IPv4 or IPv6 address */
typedef unsigned long digest;   /* md5 digest */
typedef signed char s_char;     /* precision and poll interval (log2) */

typedef struct tx {
        char    cruft;          /* leap indicator: 2bit, version number: 3bit, mode: 3bit */
        char    stratum;        /* stratum */
        char    poll;           /* poll interval */
        s_char  precision;      /* precision */
        tdist   rootdelay;      /* root delay */
        tdist   rootdisp;       /* root dispersion */
        long   refid;          /* reference ID */
        tstamp  reftime;        /* reference time */
        tstamp  org;            /* origin timestamp */
        tstamp  rec;            /* receive timestamp */
        tstamp  xmt;            /* transmit timestamp */
        long     keyid;          /* key ID */
        digest  dgst;           /* message digest */
} NtpPacket;


int main(int argc, char* argv[])
{
	char defaulthost[] = "0.de.pool.ntp.org";		// Default Hostname 
	struct hostent		*host;				// Speichert den Namen des Clientrechners
	struct protoent		*protocol;				// Speichert den Namen des Transp.Protok.
	struct sockaddr_in		sad;				// Speichert die Serveradresse
//	struct sockaddr dest_addr;
	SOCKET			sd;					// Socket Descriptor
	unsigned short	port;				// Portnummer des Servers
	char			*hostname;				// Host Name static_cast<void*>
	int				n;					// Anzahl empfangener Bytes
//	char			RecvBuf[1000];		// Puffer für Empfangsdaten

	NtpPacket	ntp_packet;

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
	fprintf(stdout, "simple NTPv4 client\n");

    memset(&sad, 0, sizeof(sad));       // Initialisiere sad mit 0
    sad.sin_family = AF_INET;           // Setze "Internetadressierung"

// Auswerten der Kommandozeilenparameter      
	if (argc > 1) {						// Falls <Rechnername> vorhanden
		hostname = argv[1];}				// dann in host speichern
	else {
		hostname = defaulthost;}				// sonst verwende "localhost"
	host = gethostbyname(hostname);	        // <Rechnername> --> IP-Adresse
	if (host == NULL) {
        fprintf(stdout,"ungültige Rechneradresse: %s\n", hostname);
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
	if (memcpy_s(&sad.sin_addr, AF_INET_LEN, host->h_addr, host->h_length)){
#else
	if (memcpy (&sad.sin_addr, host->h_addr, AF_INET_LEN) == NULL) {
#endif
		fprintf(stdout,"Fehler beim Kopieren der Rechneradresse\n");
		RNP_Cleanup();
		exit(EXIT_FAILURE);
	}
		
    if ((protocol = getprotobyname("udp")) == NULL) { // "udp" --> Protokollcode
        fprintf(stdout, "Abbildung von \"udp\" auf Protokollnummer nicht möglich!\n");
		RNP_Cleanup();
		exit(EXIT_FAILURE);}
        
// Erzeuge Socket
	sd = socket(PF_INET, SOCK_DGRAM, protocol->p_proto); 
    if (sd == INVALID_SOCKET) {
		RNP_Error(RNP_E_SOCK, "Fehler bei Generierung des Sockets");
		RNP_Cleanup();
		exit(EXIT_FAILURE);
	}

/*
	memcpy (&dest_addr.sa_data, host->h_addr, AF_INET_LEN); // TODO: errorcheck
	dest_addr.sa_family = AF_INET;
	dest_addr.sa_len = AF_INET_LEN;
*/

// ntp-packet vorbereiten
	ntp_packet.cruft = 0<<6|4<<3|3;/* li=0, v=4, mode=3 */
	ntp_packet.stratum = 0;        /* stratum */
	ntp_packet.poll = 0;           /* poll interval */
	ntp_packet.precision = 0;      /* precision */
	ntp_packet.rootdelay = 0;      /* root delay */
	ntp_packet.rootdisp = 0;       /* root dispersion */
	ntp_packet.refid = 0;          /* reference ID */
	ntp_packet.reftime = 0;        /* reference time */
	ntp_packet.org = 0;            /* origin timestamp */
	ntp_packet.rec = 0;            /* receive timestamp */
	ntp_packet.xmt = 0;            /* transmit timestamp */
	ntp_packet.keyid = 0;          /* key ID */
	ntp_packet.dgst = 0;           /* message digest */

// Eine Nachricht an den Server senden
	if ((n = sendto(sd, &ntp_packet, sizeof(ntp_packet), NO_FLAGS, (struct sockaddr *)&sad, sizeof(sad))) < 0) {
		fprintf(stdout, "Fehler beim Senden: %d\n", n);
	}

// Empfangene Daten lesen und auf dem Bildschirm schreiben
//	n = recv(sd, RecvBuf, sizeof(RecvBuf), NO_FLAGS); 

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


