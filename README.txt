Particle System API
by David McAllister
version 1.5
November 26, 2005

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
'-' to change the number of particles.

This demo shows just particles by themselves, instead of as a special
effect in an environment, but these effects can trivially be added to
any OpenGL program.

Compiling the Library for UNIX

The sample Makefile compiles both libparticle.a and pspray. If the
supplied optimization flags don't make sense, feel free to rip them
out. I've also included an SGI N32 compiled library and application.

That's really it. If you have any questions, comments, or suggestions
you can mail me at davemc@nvidia.com

