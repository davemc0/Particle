######################################################################
# Particle Spray Demo
#
# Copyright 1998 by David K. McAllister.
#
######################################################################

C++ = g++

GLUT_HOME  =/home/utils/glut-3.7
PHOME =..
DMCTOOLS_HOME =../../DMcTools

COPT = -O3

CFLAGS = $(COPT) $(COMPFLAGS) -I. -I$(PHOME) -I$(DMCTOOLS_HOME) -I$(GLUT_HOME)/include

LIBDIR =-L$(PHOME)/ParticleLib -L$(GLUT_HOME)/lib
LIBS =$(LIBDIR) -lParticle -lglut -lGL -lGLU -lXmu -lX11 -lXext -lXi -lm

OBJS = PSpray.o DrawGroups.o Effects.o Monarch.o

ALL = pspray

all: $(ALL)

.cpp.o:
	$(C++) $(CFLAGS) -c $<

pspray: $(OBJS)
	$(C++) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f $(ALL) $(OBJS)
