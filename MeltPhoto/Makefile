######################################################################
# Particle Boids Demo
#
# Copyright 1999 by David K. McAllister.
#
######################################################################

C++ = CC

GLUT_HOME =/usr/local/contrib/unmoderated
TOOLS_HOME =/afs/cs.unc.edu/project/pxfl/users/davemc/Tools
PLIB =../..

LNO =-LNO:opt=1:fusion=2:fission=2:fusion_peeling_limit=2048:cs1=32K:cs2=8M
COPT =-ptused -O3 $(LNO) -Ofast=IP27
CFLAGS = $(COPT) $(COMPFLAGS) -I$(PLIB) -I$(GLUT_HOME)/include -I$(TOOLS_HOME)

LIBDIR =-L$(PLIB)/particle -L$(TOOLS_HOME)/SGI -L$(GLUT_HOME)/lib
LIBSG =$(LIBDIR) -lparticle -lTools -ltiff -ljpeg -lglut -lGL -lGLU -lXmu -lX11 -lXext -lm

LDFLAGS =-IPA

ALL = meltphoto

all: $(ALL)

# following line needed for c++ .cc files
.SUFFIXES	: .cpp

.cpp.o:
	$(C++) $(CFLAGS) -c $<

meltphoto: MeltPhoto.o
	$(C++) $(LDFLAGS) -o $@ MeltPhoto.o $(LIBSG)

clean:
	rm -f $(ALL)
	rm -f *.o
	rm -rf ii_files rii_files
