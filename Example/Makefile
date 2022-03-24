######################################################################
# Particle Spray Demo
#
# Copyright 1998 by David K. McAllister.
#
######################################################################

C++ = g++

GLUT_HOME  =/home/utils/glut-3.7
X11_HOME =/usr/X11R6/lib

PHOME =..

COPT = -O3

CFLAGS = $(COPT) $(COMPFLAGS) -I. -I$(PHOME) -I$(GLUT_HOME)/include

LIBDIR =-L$(PHOME)/ParticleLib -L$(GLUT_HOME)/lib -L$(X11_HOME)
LIBS =$(LIBDIR) -lParticle -lglut -lGL -lGLU -lXmu -lX11 -lXext -lXi -lm

OBJS = Example.o

ALL = example

all: $(ALL)

.cpp.o:
	$(C++) $(CFLAGS) -c $<

example: $(OBJS)
	$(C++) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f $(ALL) $(OBJS)
