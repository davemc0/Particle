######################################################################
# Particle Example
#
# Copyright 1999 by David K. McAllister.
#
######################################################################

C++ = CC

PLIB =../..
GLUT_HOME =/usr/local/contrib/unmoderated

CFLAGS = $(COPT) $(COMPFLAGS) -I$(PLIB) -I$(GLUT_HOME)/include

LIBDIR =-L$(PLIB)/particle -L$(GLUT_HOME)/lib
LIBSG =$(LIBDIR) -lparticle -lglut -lGL -lGLU -lXmu -lX11 -lXext -lm

ALL = example

all: $(ALL)

# following line needed for c++ .cc files
.SUFFIXES	: .cpp

.cpp.o:
	$(C++) $(CFLAGS) -c $<

example: Example.o
	$(C++) $(LDFLAGS) -o $@ Example.o $(LIBSG)

clean:
	rm -f $(ALL)
	rm -f *.o
	rm -rf ii_files rii_files
