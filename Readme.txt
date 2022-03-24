Particle System API
by David McAllister
version 2.21
December 30, 2008

http://www.particlesystems.org

This distribution contains three source trees. Here is an explanation of each:

Particle2:  The Particle System API
            The Particle2 library is released under your choice of the following two licenses:
            * The GNU LGPL
            * You may use the code in any way you see fit, including modification, inclusion in
              other software products, repackaging, republication, etc. with the sole condition
              that credit be given to David K. McAllister and a reference to www.particlesystems.org
              be placed in the Credits or About section or other suitable user-visible location of
              any released software that includes code from the Particle System API.

Goodies:    Libs I use on Windows, like GLUT, JPEG, TIFF, PNG, Half, etc.
            None of this code was written by me. Each individual piece has a different author, a different
            copyright, and different terms of use. I am distributing these packages simply to facilitate
            building the demos. t is not part of the Particle System API, and the API itself does not
            depend on this code.

DMcTools:   This has my collection of graphics tools - matrix class, image stuff,
            3D vector class, random numbers, stop watch, VRML loader, etc.
            It is only needed by the PSpray and ParBench demo, not the API or the Example demo.
            DMcTools is copyright by me, and maybe others.


How to build:
Using Visual Studio 2008, load the DMcTools_vc90.sln solution and build it.
Then load the Particle2.sln and build it.

If you are using Visual. Studio 2005, load DMcTools_vc80.sln and build it.
Then replace all of the .vcproj files under Particle2/ with their corresponding ones in the VC8 folder.
Then load the Particle2.sln and build it.


If the demo doesn't work for you, I'd be happy to try to help. Please include as much detail as
you can about your system and the nature of the problem.

If you have any questions, code contributions or suggestions you can mail me at davemc@cs.unc.edu.
I work for Nvidia now, but don't want my Nvidia email address posted where spambots can find it.
