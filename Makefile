#
# Description:  Makefile for mpdApvDecode
#   Decode MPD and APV data from CODA 3.10 evio datafile
#

ARCH	= `uname -m`

CODA	= /daqfs/coda/3.10
EVIO_INC = ${CODA}/Linux-$(ARCH)/include
EVIO_LIB = ${CODA}/Linux-$(ARCH)/lib

CC	= g++
CFLAGS	= -I. -I${EVIO_INC} \
		-L. -L${EVIO_LIB} \
		-levio -levioxx -lrt -lexpat
CFLAGS	+= -g -Wall

PROGS	= mpdApvDecode

all: echoarch $(PROGS)

%: %.cc
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS) *~

echoarch:
	echo "Make for $(ARCH)"
