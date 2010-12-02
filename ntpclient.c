//
// simpler NTP client im Rahmen des RNP
//

#include "rnp_wrap.h"

#define JAN_1970        2208988800UL /* 1970 - 1900 in seconds */

#define PROTOPORT		123 		// Default Portnummer


/*
 * Transmit packet
 */
typedef unsigned long long tstamp;   /* NTP timestamp format */
typedef unsigned int tdist;     /* NTP short format */
typedef unsigned long ipaddr;   /* IPv4 or IPv6 address */
typedef unsigned long digest;   /* md5 digest */
typedef signed char s_char;     /* precision and poll interval (log2) */

typedef struct tx {
        char    flags;          /* leap indicator: 2bit, version number: 3bit, mode: 3bit */
        char    stratum;        /* stratum */
        char    poll;           /* poll interval */
        s_char  precision;      /* precision */
        tdist   rootdelay;      /* root delay */
        tdist   rootdisp;       /* root dispersion */
        long    refid;          /* reference ID */
        tstamp  reftime;        /* reference time */
        tstamp  org;            /* origin timestamp */
        tstamp  rec;            /* receive timestamp */
        tstamp  xmt;            /* transmit timestamp */
        long    keyid;          /* key ID */
        digest  dgst;           /* message digest */
} NtpPacket;


int main(int argc, char* argv[])
{
	char defaulthost[] = "0.de.pool.ntp.org";	// Default Hostname 
	struct hostent			*host;				// Speichert den Namen des Clientrechners
	struct protoent			*protocol;			// Speichert den Namen des Transp.Protok.
	struct sockaddr_in		server_addr;				// Speichert die Serveradresse
	SOCKET			socket_d;					// Socket Descriptor
	unsigned short	port;						// Portnummer des Servers
	char			*hostname;					// Host Name static_cast<void*>
	int				n;							// Anzahl empfangener Bytes
	NtpPacket	ntp_packet;

	RNP_Init();

//  Bildschirmausgabe einer Begrüßungsnachricht        
	fprintf(stdout, "simple NTPv4 client\n");

    memset(&server_addr, 0, sizeof(server_addr));       // Initialisiere server_addr mit 0
    server_addr.sin_family = AF_INET;           // Setze "Internetadressierung"

// Auswerten der Kommandozeilenparameter      
	if (argc > 1)						// Falls <Rechnername> vorhanden
		hostname = argv[1];				// dann in host speichern
	else
		hostname = defaulthost;				// sonst verwende "localhost"
	host = gethostbyname(hostname);	        // <Rechnername> --> IP-Adresse
	if (host == NULL) {
        fprintf(stdout,"ungültige Rechneradresse: %s\n", hostname);
		RNP_Cleanup();
		exit(EXIT_FAILURE);
	}
		
    if (argc > 2)                       // Falls <Portnummer> vorhanden
		port = atoi(argv[2]);			// dann konvertiere nach int
	else 
		port = PROTOPORT;				// sonst verwende Default-Wert    
	
    if (port > 0)            
		server_addr.sin_port = htons(port);		// Host --> Netzwerk-Byteordnung
	else {								// Drucke Fehlermeldung; Programmende
		fprintf(stdout,"Ungültige Portnummer %s\n", argv[2]);
		RNP_Cleanup();
		exit(EXIT_FAILURE);
	}
         
// datenstrukturen initialisieren
//	if(memcpy(&server_addr.sin_addr, host->h_addr, AF_INET_LEN) == NULL) {
	if(RNP_copy_hostaddr_to_sockaddr(&server_addr, host)) {
		fprintf(stdout,"Fehler beim Kopieren der Rechneradresse\n");
		RNP_Cleanup();
		exit(EXIT_FAILURE);
	}
		
    if ((protocol = getprotobyname("udp")) == NULL) { // "udp" --> Protokollcode
        fprintf(stdout, "Abbildung von \"udp\" auf Protokollnummer nicht möglich!\n");
		RNP_Cleanup();
		exit(EXIT_FAILURE);
	}
        
// Erzeuge Socket
	socket_d = socket(PF_INET, SOCK_DGRAM, protocol->p_proto); 
    if (socket_d == INVALID_SOCKET) {
		RNP_Error(RNP_E_SOCK, "Fehler bei Generierung des Sockets");
		RNP_Cleanup();
		exit(EXIT_FAILURE);
	}

// ntp-packet vorbereiten
	ntp_packet.flags = 0<<6|4<<3|3;/* li=0, v=4, mode=3 */
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
	n = sendto(socket_d, (const char *)&ntp_packet, sizeof(ntp_packet), NO_FLAGS, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (n < 0) {
		fprintf(stdout, "Fehler beim Senden: %d\n", n);
	}
	else {
		fprintf(stdout, "OK: %d byte gesendet.\n", n);
	}

// und eine Nachricht vom Server empfangen
	n = recvfrom(socket_d, (char *)&ntp_packet, sizeof(ntp_packet), NO_FLAGS, (struct sockaddr *)&server_addr, (unsigned int *)&n);
	if (n < 0) {
		fprintf(stdout, "Fehler beim empfangen: %d\n", n);
	}
	else {
		fprintf(stdout, "OK: %d byte empfangen.\n", n);
	}


	RNP_Close(socket_d);
	exit(EXIT_SUCCESS);
} // main


