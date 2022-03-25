# Particle
The Particle System API, since 1999, using C++, OpenGL, GLUT, and CMake

by David McAllister
version 3.0 alpha 1
June 2009

Was previously hosted at: http://www.particlesystems.org
Now on https://github.com/davemc0/Particle.git

=================
The big idea with this release is to emit CUDA source code for an action list,
then compile that with CUDA and at app run time execute the generated action list on the device

It's not done and doesn't work very well. Only the Example test runs so far. Lots of porting and coding to do.
=================

Building ParticleLib can be accomplished using just this code, CMake, and a C++ compiler.
It has no external dependencies.

However, the examples and demos also depend on DMcTools: https://github.com/davemc0/DMcTools.git

DMcTools has my collection of graphics tools - matrix class, image stuff,
3D vector class, random numbers, timer, 3D model loader, etc.
It is only needed by the PSpray and ParBench demo, not the API or the Example demo.
DMcTools is copyright by me, and maybe others.

To build everything you will also need:
CMake 3.20 or newer. (You could try an older one, but no guarantees.)
FreeGLUT. I use 3.2.2.
GLEW. I use 2.2.0.

How to build:
In the main directory, do something like:
mkdir build ; cd build ; cmake .. -G "Visual Studio 16 2019" -A x64 ; cd ..

Then you can build from the command line:
cmake --build build/ --config Release
or launch build/ParticleSolution.sln in Visual Studio.

=================

If the demos don't work for you, I'd be happy to try to help. Please include as much detail as
you can about your system and the nature of the problem.

If you have any questions, code contributions or suggestions use github issues or contact me however.