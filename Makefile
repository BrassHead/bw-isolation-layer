# BW isolation layer Linux makefile
#
# Copyright (C) 1999-2013 Brian Bray
#
#
# Usage:
#
# make				Makes the library libbw.a
# make check		Compiles and runs basic test suite
# make clean		Cleans the source tree
# make dist			Make distribution tarball
# make docgen		Updates the docgen HTML output in ./doc/auto
# make restyle      Uniform indent and brace coding style
#

PACKAGE = bw
VERSION = 0.9

CXX = c++
AR = ar
RANLIB = ranlib
MAKE = make

CXXOPTS = -g -D_DEBUG
CCFLAGS = -std=c++11 -I./include -Wall -Werror $(DEFS)

%.o: %.cc
	$(CXX) -c $(CXXOPTS) $(CCFLAGS) $<

BASICSOURCES = bwassert.cc exception.cc file.cc string.cc ustring.cc \
	filename.cc directory.cc html.cc http.cc \
	logging.cc custom.cc xml.cc

GUISOURCES = main.cc process.cc guiexception.cc context.cc figure.cc \
	ffigure.cc parent.cc figurebase.cc frame.cc scene.cc size.cc \
	mouse.cc focus.cc button.cc canvas.cc \
	pen.cc linetool.cc tool.cc brush.cc press.cc font.cc \
	styletools.cc colour.cc bgc.cc gdevice.cc \

SQLSOURCES = sql.cc
TRIALSOURCES = xiso.cc 

BASICOBJS = bwassert.o exception.o file.o string.o ustring.o \
	filename.o directory.o html.o http.o \
	logging.o custom.o xml.o

GUIOBJS = main.o process.o guiexception.o context.o figure.o \
	ffigure.o parent.o figurebase.o frame.o scene.o size.o \
	mouse.o focus.o button.o canvas.o \
	pen.o linetool.o tool.o brush.o press.o font.o \
	styletools.o colour.o bgc.o gdevice.o \

SQLOBJS = sql.o
TRIALOBJS = xiso.o 

SOURCES = $(BASICSOURCES)
OBJECTS = $(BASICOBJS)
SOURCES += $(SQLSOURCES)
OBJECTS += $(SQLOBJS)
SOURCES += $(TRIALSOURCES)
OBJECTS += $(TRIALOBJS)
SOURCES += $(GUISOURCES)
OBJECTS += $(GUIOBJS)

# targets

all: libbw.a
	( cd test ; $(MAKE) )

libbw.a: $(OBJECTS)
	rm -f libbw.a
	$(AR) cru libbw.a $(OBJECTS)
	$(RANLIB) libbw.a

check: libbw.a
	( cd test ; $(MAKE) check )

clean:
	( cd test ; $(MAKE) clean )
	rm -f *.o
	rm -f *~ include/*~ include/bw/*~ doc/*~
	rm -f core
	rm -f libbw.a
	rm -f doc/auto/*
	
dist: docgen
	( \
	  cd .. ; \
	  tar czf $(PACKAGE)-$(VERSION).`date +%Y%m%d`.tgz $(PACKAGE) ; \
	)

docgen: $(SOURCES)
	rm -f doc/auto/*
	docgen doc/auto *.cc

restyle:
	astyle --recursive --style=stroustrup --indent=tab=4 "*.cc" "*.h"


.PHONY: docgen all clean dist release restyle

# Xlib implementation
bgc.o:		bgc.cc bgc.h event.h ffigure.h fcanvas.h gdevice.h \
				include/bw/bwassert.h include/bw/string.h include/bw/tools.h include/bw/figure.h include/bw/context.h
brush.o:	bgc.h include/bw/bwassert.h include/bw/string.h include/bw/tools.h include/bw/figure.h include/bw/context.h \
                fcolour.h event.h ffigure.h fcanvas.h
canvas.o:	canvas.cc include/bw/bwassert.h include/bw/string.h include/bw/tools.h include/bw/context.h include/bw/figure.h event.h fcanvas.h ffigure.h
colour.o:	colour.cc include/bw/bwassert.h include/bw/string.h include/bw/tools.h fcolour.h
context.o:	context.cc include/bw/context.h
ffigure.o:	ffigure.cc ffigure.h event.h include/bw/bwassert.h include/bw/string.h include/bw/context.h include/bw/exception.h include/bw/figure.h
figure.o:	figure.cc include/bw/trace.h include/bw/bwassert.h include/bw/string.h include/bw/context.h \
                include/bw/tools.h include/bw/figure.h ffigure.h event.h
figurebase.o:	figurebase.cc include/bw/bwassert.h include/bw/string.h include/bw/figure.h event.h
focus.o:	focus.cc include/bw/bwassert.h include/bw/string.h include/bw/context.h include/bw/figure.h event.h ffigure.h
font.o:		font.cc include/bw/bwassert.h include/bw/string.h include/bw/figure.h \
                include/bw/scene.h include/bw/tools.h include/bw/context.h \
				event.h ffigure.h fcanvas.h ffont.h gdevice.h
frame.o:	frame.cc include/bw/bwassert.h include/bw/string.h include/bw/exception.h include/bw/process.h \
                    include/bw/figure.h include/bw/context.h include/bw/scene.h event.h ffigure.h fscene.h
gdevice.o:	gdevice.cc include/bw/bwassert.h include/bw/string.h include/bw/tools.h gdevice.h
linetool.o: linetool.cc include/bw/bwassert.h include/bw/string.h include/bw/tools.h bgc.h
main.o:     main.cc include/bw/trace.h include/bw/bwassert.h include/bw/exception.h include/bw/process.h fprocess.h
mouse.o:    mouse.cc include/bw/bwassert.h include/bw/string.h include/bw/figure.h include/bw/context.h event.h ffigure.h
parent.o:   parent.cc include/bw/bwassert.h include/bw/string.h include/bw/figure.h include/bw/context.h ffigure.h event.h
pen.o:      pen.cc include/bw/bwassert.h include/bw/string.h include/bw/tools.h include/bw/figure.h include/bw/context.h \
                bgc.h fcolour.h event.h ffigure.h fcanvas.h
press.o:    press.cc include/bw/bwassert.h include/bw/string.h include/bw/tools.h include/bw/figure.h include/bw/context.h \
                bgc.h ffont.h fcolour.h event.h ffigure.h fcanvas.h
process.o:  process.cc include/bw/scene.h include/bw/string.h include/bw/exception.h include/bw/bwassert.h \
                include/bw/process.h include/bw/figure.h include/bw/context.h \
                fprocess.h event.h ffigure.h
scene.o:    scene.cc include/bw/trace.h include/bw/bwassert.h include/bw/string.h include/bw/figure.h \
                include/bw/scene.h include/bw/exception.h include/bw/process.h \
                include/bw/context.h include/bw/tools.h fscene.h fprocess.h ffigure.h event.h gdevice.h
size.o:     size.cc include/bw/bwassert.h include/bw/string.h include/bw/figure.h include/bw/context.h event.h ffigure.h
tool.o:     tool.cc include/bw/bwassert.h include/bw/string.h include/bw/tools.h bgc.h fcolour.h
xiso.o:     xiso.cc include/bw/bwiso.h include/bw/custom.h include/bw/bwassert.h include/bw/trace.h include/bw/countable.h


# Isolated from Xlib
button.o:	button.cc include/bw/bwassert.h include/bw/string.h include/bw/figure.h include/bw/trace.h \
                include/bw/button.h include/bw/tools.h include/bw/styletools.h
bwassert.o:	bwassert.cc include/bw/bwassert.h include/bw/string.h include/bw/bwassert.h include/bw/trace.h include/bw/exception.h
custom.o:	custom.cc include/bw/custom.h include/bw/exception.h include/bw/process.h
directory.o:	directory.cc include/bw/exception.h include/bw/bwassert.h include/bw/string.h \
                    include/bw/filename.h include/bw/directory.h
exception.o:	exception.cc include/bw/bwassert.h include/bw/exception.h
file.o:		file.cc include/bw/file.h include/bw/exception.h include/bw/bwassert.h
filename.o:	filename.cc include/bw/bwassert.h include/bw/filename.h include/bw/exception.h include/bw/string.h
guiexception.o:	guiexception.cc include/bw/exception.h include/bw/bwassert.h
html.o:		html.cc include/bw/html.h include/bw/bwassert.h include/bw/string.h
http.o:     http.cc include/bw/trace.h include/bw/http.h include/bw/exception.h include/bw/bwassert.h include/bw/string.h
logging.o:  logging.cc include/bw/logging.h include/bw/bwassert.h include/bw/string.h
sql.o:      sql.cc include/bw/sql.h
string.o:   string.cc include/bw/bwassert.h include/bw/string.h
styletools.o:   styletools.cc include/bw/bwassert.h include/bw/string.h include/bw/tools.h include/bw/styletools.h
ustring.o:  ustring.cc include/bw/bwassert.h include/bw/ustring.h
xml.o:      xml.cc include/bw/trace.h include/bw/bwassert.h include/bw/countable.h include/bw/exception.h include/bw/xml.h

