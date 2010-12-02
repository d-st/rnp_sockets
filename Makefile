#
# Makefile for RNP Socket programming
#

CC = gcc
CFLAGS = -Wall
LDFLAGS = 
LDLIBS = 

COMMON = serror.c
COMMONOBJS = $(patsubst %.c,%.o,$(COMMON))
SERVERSRCS = server.c
CLIENTSRCS = client.c
SERVEROBJS = $(patsubst %.c,%.o,$(SERVERSRCS))
CLIENTOBJS = $(patsubst %.c,%.o,$(CLIENTSRCS))
SERVERBIN = server
CLIENTBIN = client

NTPSRCS = ntpclient.c
NTPOBJS =  $(patsubst %.c,%.o,$(NTPSRCS))
NTPCLIENT = ntpclient

.PHONY:		all tests depend clean dist-clean

all:		$(SERVERBIN) $(CLIENTBIN) $(NTPCLIENT)

$(SERVERBIN):		$(SERVEROBJS) $(COMMONOBJS)
		$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(CLIENTBIN):		$(CLIENTOBJS) $(COMMONOBJS)
		$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(NTPCLIENT):		$(NTPOBJS) $(COMMONOBJS)
		$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o:		%.c
		$(CC) $(CFLAGS) -o $@ -c $<

clean:
		rm -f *~ *.o

dist-clean:	clean
		rm -f $(SERVERBIN) $(CLIENTBIN)
