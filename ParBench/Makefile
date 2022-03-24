######################################################################
# Particle Benchmark
#
# Copyright 2006 by David K. McAllister.
#
######################################################################

C++ = g++

PHOME =..
DMCTOOLS_HOME =../../DMcTools

COPT = -O3

CFLAGS = -DNO_OGL_OBSTACLES $(COPT) $(COMPFLAGS) -I. -I$(PHOME) -I$(DMCTOOLS_HOME)

LIBDIR =-L$(PHOME)/ParticleLib -L$(DMCTOOLS_HOME)/Release_i686
LIBS =$(LIBDIR) -lParticle -lDMcTools -lm

OBJS = ParBench.o Effects.o

ALL = parbench

all: $(ALL)

.cpp.o:
	$(C++) $(CFLAGS) -c $<

Effects.o:
	$(C++) $(CFLAGS) -c ../PSpray/Effects.cpp

parbench: $(OBJS)
	$(C++) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f $(ALL) $(OBJS)
