############################################################################
# Makefile for vgrep
#
#   Revision 1.00 2006 Lechee
#   Initial version
#
#
############################################################################
CC = gcc
LD = gcc
CFLAGS = -O2 -Wall -c 
LDFLAGS = -O2 -o

DEL = rm

all:	vgrep$(EXE)

vgrep:	vgrep.o
	$(LD) $^ $(LDFLAGS) $@

vgrep.o:vgrep.c
	$(CC) $(CFLAGS) $<

install:
	cp vgrep /usr/local/bin/vgrep
clean:
		$(DEL) *.o
		$(DEL) vgrep
		
