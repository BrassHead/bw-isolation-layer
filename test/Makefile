# Makefile for the test directory of BW
#
# Copyright (C) 1998-2013 Brian Bray
#

SHELL = /bin/sh
CXX = c++
CXXOPTS = -g -D_DEBUG
CCFLAGS = -std=c++11 -I../include -Wall -Werror $(DEFS)
LDFLAGS = -Wl,-rpath,/usr/X11R6/lib -L/usr/X11R6/lib
LIBS = ../libbw.a -lX11

%: %.cc ../libbw.a
	$(CXX) $(CXXOPTS) $(CCFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)


TESTPROGS = button1 bwhi string1 string2 bwiso1 bwisohi cptr1 \
				filename1 ini1 xml1
TESTSOURCES = button1.cc bwhi.cc string1.cc string2.cc bwiso1.cc bwisohi.cc cptr1.cc \
                filename1.cc ini1.cc xml1.cc
XISOOBJS = ../xiso.o ../string.o ../bwassert.o

all:	$(TESTPROGS)

bwiso1: bwiso1.cc $(XISOOBJS)
	$(CXX) $(CXXOPTS) $(CCFLAGS) $(LDFLAGS) -o $@ $< $(XISOOBJS) -lX11 -lXext

bwisohi: bwisohi.cc $(XISOOBJS)
	$(CXX) $(CXXOPTS) $(CCFLAGS) $(LDFLAGS) -o $@ $< $(XISOOBJS) -lX11 -lXext


check:	$(TESTPROGS)
	bash runtest.sh

clean:
	rm -f *~
	-rm -f $(TESTPROGS)
	-rm -f *~

.PHONY: all check clean

