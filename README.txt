Particle System API
by David McAllister
version 1.21
February 23, 2001

http://www.cs.unc.edu/~davemc/Particle

For some of these demos you must download and use

  http://www.cs.unc.edu/~davemc/Stuff/Goodies.zip

which has compiled JPEG, TIFF, PNG, and GLUT libraries and 

  http://www.cs.unc.edu/~davemc/DMcTools

which is my collection of graphics tools - VRML loader, image stuff,
vector class, random number generator stop watch, matrix class, ...
Also, you will need to download
http://www.cs.unc.edu/~davemc/Stuff/Goodies.tar.gz to get the TIFF,
JPEG, and GLUT libraries and includes.

Running the MeltingPhoto Demo

Drag any image (GIF, TIFF, JPEG, ...) onto the MeltingPhoto icon or
specify the image's filename on the command line. When the program
starts it will show your photo. Press 'C' or 'A' or 'J' or some other
effect button to stir up the pixels of the image, then press 'R' to
make it come back together. Press 'S' to start over.

Running the PSpray Demo

To run the PSpray demo, click on pspray.exe and the demo will start by
drawing a random demo. It uses motion blur, which may be faster or
slower than no motion blur. To toggle it, press 'm'. Press the right
mouse button to see a menu of options. Many different particle effects
are included in the demo. The menu tells what they are. Press the
space bar to cause an explosion. Press 'f' to toggle full screen mode.

By default, the program runs a maximum of 10000 particles. Use '+' and
'-' to change the number of particles. I have a 400 MHz Pentium ][ and
it can run at about 20 frames per second with 10000 particles. That's
using an Evans & Sutherland RealImage1000 graphics card, which doesn't
have a geometry accelerator. If you have a geometry accelerator, I bet
you could do twice as many particles because the particle system code
does less work per particle than OpenGL does.

This demo shows just particles by themselves, instead of as a special
effect in an environment, but these effects can trivially be added to
any OpenGL program.

Compiling the Library for UNIX

The sample Makefile compiles both libparticle.a and pspray. If the
supplied optimization flags don't make sense, feel free to rip them
out. I've also included an SGI N32 compiled library and application.

Compiling the Library for Evil Windows

The particle library, particle.lib, comes precompiled in release
mode. If you want to recompile the library (to modify it, say), create
a new Win32 Static Library project called Particle in Visual Studio
and add all the .cpp and .h files to the project, except
pspray.cpp. Then just compile it. I've included particle.dsp, which is
a sample project file. You can try just adding it to your workspace
instead of making a new project.

Compiling the Demo for Evil Windows

Make a new Win32 Console Application project called PSpray. Make it
depend on the project Particle. Add pspray.cpp to the project. In the
C++ setting, add the particle system library folder to the include
path, and GLUT if necessary. Then in Link settings, add particle.lib,
glut32.lib, glu32.lib, and opengl32.lib.

That's really it. If you have any questions, comments, or suggestions
you can mail me at davemc@cs.unc.edu
