######################################################################
# Particle Spray Demo
#
# Copyright 1998 by David K. McAllister.
#
######################################################################

C++ = CC

GLUT_HOME =/usr/local/contrib/unmoderated
PLIB =../..

LNO =-LNO:opt=1:fusion=2:fission=2:fusion_peeling_limit=2048:cs1=32K:cs2=8M
COPT =-ptused -O3 $(LNO) -Ofast=IP27
CFLAGS = $(COPT) $(COMPFLAGS) -I$(PLIB) -I$(GLUT_HOME)/include

LIBDIR =-L$(PLIB)/particle -L$(GLUT_HOME)/lib
LIBSG =$(LIBDIR) -lparticle -lglut -lGL -lGLU -lXmu -lX11 -lXext -lm
# -lmp XXX

LDFLAGS =-IPA

ALL = pspray

all: $(ALL)

# following line needed for c++ .cc files
.SUFFIXES	: .cpp

.cpp.o:
	$(C++) $(CFLAGS) -c $<

pspray: PSpray.o
	$(C++) $(LDFLAGS) -o $@ PSpray.o $(LIBSG)

clean:
	rm -f $(ALL)
	rm -f *.o
