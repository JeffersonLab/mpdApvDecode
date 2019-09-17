#
# Description:  Makefile for mpdApvDecode
#   Decode MPD and APV data from CODA 3.10 evio datafile
#

ARCH=$(shell uname -m)

CODA	= /daqfs/coda/3.10
EVIO_INC = ${CODA}/Linux-$(ARCH)/include
EVIO_LIB = ${CODA}/Linux-$(ARCH)/lib

CC	= g++
CFLAGS	= -I. -I${EVIO_INC} \
		-L. -L${EVIO_LIB} \
		-levio -levioxx -lrt -lexpat
CFLAGS	+= -g -Wall

PROG	= mpdApvDecode
SRC     = $(wildcard *.cc)
OBJ     = ${SRC:.cc=.o}

all: echoarch $(PROG)

$(PROG): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.cc
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(PROG) *~

echoarch:
	@echo "Make for $(ARCH)"
