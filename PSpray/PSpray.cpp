// PSpray.cpp
//
// Copyright 1998-2006 by David K. McAllister
//
// This application demonstrates particle systems for interactive graphics.
// It uses OpenGL and GLUT.

// Texture and light the butterflies
// Texture the rain drops
// Fix motion blur
// Key to adjust point diameter
// Multisampling
// Multiple textures
// Fountain playground
// When in random mode reset all rendering params to best for that demo
//   motion blur, kill before start, primitive, texture, depth test, speed, particle size
// Do I just make an init function, or do I make a struct?

#include "../PSpray/DrawGroups.h"
#include "../PSpray/Effects.h"
#include "../PSpray/Monarch.h"

#include "Particle/pAPI.h"

// The following header files are part of DMcTools.
// DMcTools is part of the same source distribution as the Particle API.
#include "Util/Timer.h"
#include "Util/Utils.h"
#include "Util/Assert.h"

#define GLH_EXT_SINGLE_FILE
#include "glh_extensions.h"
#include "GL/glut.h"

#include <iostream>
#include <string>
#include <cmath>
#include <cstring>
#include <ctime>

using namespace std;

#ifdef WIN32
#pragma warning (disable:4305) /* disable bogus conversion warnings */
#define drand48() (((float) rand())/((float) RAND_MAX))
#define lrand48() ((rand() << 15) ^ rand())
#define srand48(x) srand(x)
#endif

static bool MotionBlur = false, FreezeParticles = false, AntiAlias = true, DepthTest = false;
static bool ConstColor = false, ShowText = true, ParticleCam = false, SortParticles = false, SphereTexture = true;
static bool Immediate = false, DrawGround = false, CameraMotion = true, FullScreen = false, PointSpritesAllowed = true;
static int DemoNum = 10, PrimType = GL_POINTS, DisplayListID = -1, SpotTexID = -1;
static const float DEMO_MIN_SEC = 6.0f;
static int RandomDemo = 500; // A one-in-this-many chance of changing demos this frame
static float BlurRate = 0.09;
static char *PrimName = "Points";
static char *FName = NULL;

static Timer Clock, RandomDemoClock;
static ParticleContext_t P;
static ParticleEffects Efx(P, 40000);

void menu(int);

static void SetupBenchmark()
{
    srand48(531);
    Immediate = false;
    CameraMotion = false;
    SortParticles = false;
    DemoNum = 22; // tornado
    if(RandomDemo > 0) RandomDemo = -RandomDemo;
    // RandomDemo = 10; // Try to do PGO on lots of effects.
}

// Symmetric gaussian centered at origin.
// No covariance matrix. Give it X and Y.
inline float Gaussian2(float x, float y, float sigma)
{
    // The sqrt of 2 pi.
#define MY_SQRT2PI 2.506628274631000502415765284811045253006
    return exp(-0.5 * (x*x + y*y) / (sigma*sigma)) / (MY_SQRT2PI * sigma);
}

void MakeGaussianSpotTexture()
{
    const int DIM = 32;
    const int DIM2 = 16;
    const float TEX_SCALE = 6.0;

    glGenTextures(1, (GLuint *)&SpotTexID);
    glBindTexture(GL_TEXTURE_2D, SpotTexID);

    float *img = new float[DIM*DIM];

    for(int y=0; y<DIM; y++) {
        for(int x=0; x<DIM; x++) {
            // Clamping the edges to zero allows Nvidia's blend optimizations to do their thing.
            if(x==0 || x==DIM-1 || y==0 || y==DIM-1)
                img[y*DIM+x] = 0;
            else {
                img[y*DIM+x] = TEX_SCALE * Gaussian2(x-DIM2, y-DIM2, (DIM*0.15));
            }
        }
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    GL_ASSERT();

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA16, DIM, DIM, GL_ALPHA, GL_FLOAT, img);
}

void MakeSphereTexture()
{
    const int DIM = 128;
    const int DIM2 = 63;
    const float TEX_SCALE = 6.0;

    glGenTextures(1, (GLuint *)&SpotTexID);
    glBindTexture(GL_TEXTURE_2D, SpotTexID);

    float *img = new float[DIM*DIM];

    pVec light(1,1,3);
    light.normalize();

    for(int y=0; y<DIM; y++) {
        for(int x=0; x<DIM; x++) {
            // Clamping the edges to zero allows Nvidia's blend optimizations to do their thing.
            if(x==0 || x==DIM-1 || y==0 || y==DIM-1)
                img[y*DIM+x] = 0;
            else {
                pVec p(x, y, 0);
                p -= pVec(DIM2, DIM2, 0);
                float len = p.length();
                float z = sqrt(DIM2*DIM2 - len*len);
                p.z() = z;
                if(len >= DIM2) {
                    img[y*DIM+x] = 0.0;
                    continue;
                }

                p.normalize();
                float v = dot(p, light);
                if(v < 0) v = 0;

                img[y*DIM+x] = v;
            }
        }
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    GL_ASSERT();

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA16, DIM, DIM, GL_ALPHA, GL_FLOAT, img);
}

static void showBitmapMessage(GLfloat x, GLfloat y, GLfloat z, char *message)
{
    if(message == NULL)
        return;

    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glRasterPos2f(x, y);
    while (*message) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message);
        message++;
    }

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPopAttrib();
}

void InitProgs()
{
    bool GotExt = true;
    // GotExt = glh_init_extensions("GL_ARB_multitexture") && GotExt;
    // GotExt = glh_init_extensions("GL_NV_vertex_program") && GotExt;
    // GotExt = glh_init_extensions("GL_NV_fragment_program") && GotExt;
    // GotExt = glh_init_extensions("WGL_ARB_pixel_format") && GotExt;
    GotExt = glh_init_extensions("GL_ARB_point_parameters") && GotExt;
    GotExt = glh_init_extensions("GL_ARB_point_sprite") && GotExt;
    PointSpritesAllowed = GotExt;
    if(PointSpritesAllowed) menu('p');

    // Make the point size attenuate with distance.
    // These numbers are arbitrary and need to be fixed for accuracy.
    // The most correct way to do this is to compute the determinant of the upper 3x3 of the
    // ModelView + Viewport matrix. This gives a measure of the change in size from model space
    // to eye space. The cube root of this estimates the 1D change in scale. Divide this by W
    // per point.
    float params[3] = {0.0f, 0.0f, 0.00003f};
    glPointParameterfvARB(GL_POINT_DISTANCE_ATTENUATION_ARB, params);
    glPointParameterfARB(GL_POINT_SIZE_MIN_ARB, 0);
    glPointParameterfARB(GL_POINT_SIZE_MAX_ARB, 5000);
    glPointParameterfARB(GL_POINT_FADE_THRESHOLD_SIZE_ARB, 1);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glLineWidth(1.0);
    glPointSize(1.0);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 2/255.0);

    if(DepthTest)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0, 0.0, 0.0, 0.0);

    // Texture unit state
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    float col[4] = {1.f, 1.f, 1.f, 1.f};
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, col);
    glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);

    if(SphereTexture)
        MakeSphereTexture();
    else
        MakeGaussianSpotTexture();

    DisplayListID = glGenLists(1);
#if 1
    Monarch(DisplayListID);
#else
    glNewList(DisplayListID, GL_COMPILE);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 1, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 1);
    glEnd();
    glEndList();
#endif
}

void Draw()
{
    static int CameraSystem = -1;
    static bool Init = false;
    if(!Init) {
        Init = true;
        InitProgs();

        // Use a particle to model the camera motion
        CameraSystem = P.GenParticleGroups(1, 1);
        P.CurrentGroup(CameraSystem);
        P.Velocity(PDSphere(pVec(0, 0, 0), 0.06, 0.06));
        P.Vertex(pVec(0,-19,15));
    }

    glLoadIdentity();

    if(MotionBlur) {
        // This is a cheezy motion blur that dims the old frame contents
        // before rendering the new ones. Requires single-buffering.
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

        glDrawBuffer(GL_FRONT);
        glColor4f(0, 0, 0, BlurRate);
        glRectf(-1, -1, 1, 1);
        if(DepthTest) glClear(GL_DEPTH_BUFFER_BIT);

        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    } else {
        glClear(GL_COLOR_BUFFER_BIT | (DepthTest ? GL_DEPTH_BUFFER_BIT:0));
    }

    // Use a particle to model the camera motion
    P.CurrentGroup(CameraSystem);
    if(CameraMotion) {
        P.Bounce(0, 1, 0.1, PDSphere(pVec(0, -10, 7), 15));
        P.Move();
    }

    // Make the camera track a particle
    if(ParticleCam)
        P.CurrentGroup(Efx.particle_handle);

    pVec Cam, Vel;
    P.GetParticles(0, 1, (float *)&Cam, NULL, (float *)&Vel);

#if 0
    pVec At=Cam+Vel;
#else
    pVec At(0,0,3);
#endif

    gluLookAt(Cam.x(), Cam.y(), Cam.z(), At.x(), At.y(), At.z(), 0, 0, 1);

    if(DrawGround) {
        glColor3f(0,0.8,0.2);
        glBegin(GL_TRIANGLE_STRIP);
        glVertex3f(-10,-10,0);
        glVertex3f(-10,10,0);
        glVertex3f(10,-10,0);
        glVertex3f(10,10,0);
        glEnd();
    }

    // Do the particle dynamics
    if(!FreezeParticles) {
        P.CurrentGroup(Efx.particle_handle);
        for(int step = 0; step < Efx.numSteps; step++) {
            Efx.CallDemo(DemoNum, false, Immediate);
        }
    }

    P.CurrentGroup(Efx.particle_handle);
    if(SortParticles) {
        pVec Look = DepthTest ? (At - Cam) : (Cam - At);
        P.Sort(Cam, Look);
    }

    if(PrimType == 0x0103) {
        DrawGroupAsDisplayLists(P, DisplayListID, ConstColor, false);
        // The cross product of the velocity vector and the previous frame's velocity vector
        // can give us a vector pointing to the side. Use this to orient the particles.
        P.CopyVertexB(false, true);
    } else if(PrimType == GL_LINES) {
        DrawGroupAsLines(P, ConstColor);
    } else if(PrimType == GL_POINTS) {
        DrawGroupAsPoints(P, ConstColor);
    } else if(PrimType == 0x102) { // Point sprites
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_POINT_SPRITE_ARB);
        DrawGroupAsPoints(P, ConstColor);
        glDisable(GL_POINT_SPRITE_ARB);
        glDisable(GL_TEXTURE_2D);
    } else if(PrimType == 0x100) {
        pVec view = At - Cam;
        view.normalize();
        pVec up(0, 0, 1);
        glEnable(GL_TEXTURE_2D);
        DrawGroupAsTriSprites(P, view, up, 0.16, true, true, ConstColor);
        glDisable(GL_TEXTURE_2D);
    } else if(PrimType == 0x101) {
        pVec view = At - Cam;
        view.normalize();
        pVec up(0, 0, 1);
        glEnable(GL_TEXTURE_2D);
        DrawGroupAsQuadSprites(P, view, up, 0.16, true, true, ConstColor);
        glDisable(GL_TEXTURE_2D);
    } else {
        // Don't draw.
    }

    GL_ASSERT();

    // Draw the text.
#define NUM_FRAMES_TO_AVG_FOR_CLOCK 30
    static double ClockTime = 1.0;

    if(ShowText) {
        glLoadIdentity();
        glColor3f(1, 0, 0);
        char msg[256];
        float fps = double(NUM_FRAMES_TO_AVG_FOR_CLOCK) / ClockTime;
        int cnt = (int)P.GetGroupCount();

        sprintf(msg, " %c%c%c%c%c%c%c%c n=%5d fps=%02.2f %s %s",
            MotionBlur ? 'B':' ',
            FreezeParticles ? 'F':' ',
            AntiAlias ? 'A':' ',
            (RandomDemo > 0) ? 'R':' ',
            DepthTest ? 'D':' ',
            Immediate ? 'I':' ',
            CameraMotion ? 'C':' ',
            SortParticles ? 'S':' ',
            cnt, fps, PrimName, Efx.GetCurEffectName());

        showBitmapMessage(-0.95f, 0.85f, 0.0f, msg);
    }

    static int FrameCountForClock = 0;
    FrameCountForClock++;
    if(FrameCountForClock >= NUM_FRAMES_TO_AVG_FOR_CLOCK)
    {
        ClockTime = Clock.Reset();
        Clock.Start();
        FrameCountForClock = 0;
    }

    //  cerr << FrameCountForClock << " " << cnt << " " << fps << " " << ClockTime << endl;

    if(!MotionBlur)
        glutSwapBuffers();

    // Change to a different random demo
    if(RandomDemo > 0 && ((lrand48() % RandomDemo) == 0) && RandomDemoClock.Read() > DEMO_MIN_SEC) {
        RandomDemoClock.Reset();
        DemoNum = Efx.CallDemo(lrand48(), true, Immediate);
    }
}

void Reshape(int w, int h)
{
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40, w / double(h), 1, 100);
    glMatrixMode(GL_MODELVIEW);

    // Useful for motion blur so background doesn't get ugly.
    glClear(GL_COLOR_BUFFER_BIT | (DepthTest ? GL_DEPTH_BUFFER_BIT:0));
}

void menu(int item)
{
    static int OldWidth, OldHeight;

    switch(item) {
    case ' ':
        DemoNum = Efx.CallDemo(10, true, Immediate); // Explosion
        break;
    case GLUT_KEY_UP + 0x1000:
        DemoNum = Efx.CallDemo(13, true, Immediate); // Restore
        break;
    case GLUT_KEY_DOWN + 0x1000:
        DemoNum = Efx.CallDemo(lrand48(), true, Immediate);
        break;
    case GLUT_KEY_LEFT + 0x1000:
        DemoNum--;
        DemoNum = Efx.CallDemo(DemoNum, true, Immediate);
        break;
    case GLUT_KEY_RIGHT + 0x1000:
        DemoNum++;
        DemoNum = Efx.CallDemo(DemoNum, true, Immediate);
        break;
    case 'i':
        Immediate = !Immediate;
        cerr << "Switching to " << (Immediate ? "Immediate":"Action list") << " mode.\n";
        Efx.CallDemo(DemoNum, true, Immediate);
        break;
    case 'm':
        MotionBlur = !MotionBlur;
        if(!MotionBlur)
            glDrawBuffer(GL_BACK);
        break;
    case 'a':
        AntiAlias = !AntiAlias;
        if(AntiAlias) {
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_POINT_SMOOTH);
        } else {
            glDisable(GL_LINE_SMOOTH);
            glDisable(GL_POINT_SMOOTH);
        }
        break;
    case 'c':
        CameraMotion = !CameraMotion;
        break;
    case 'r':
        RandomDemo *= -1;
        break;
    case 'f':
        FullScreen = !FullScreen;
        if(FullScreen) {
            OldWidth = glutGet(GLenum(GLUT_WINDOW_WIDTH));
            OldHeight = glutGet(GLenum(GLUT_WINDOW_HEIGHT));
            glutSetCursor(GLUT_CURSOR_NONE);
            glutFullScreen();
        } else {
            glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
            glutReshapeWindow(OldWidth, OldHeight);
        }
        break;
    case 'p':
        if(PrimType == GL_POINTS && PointSpritesAllowed) {
            PrimType = 0x0102;
            PrimName = "Point Sprites";
        } else if(PrimType == 0x0102 || PrimType == GL_POINTS) {
            PrimType = GL_LINES;
            PrimName = "Lines";
        } else if(PrimType == GL_LINES) {
            PrimType = 0x0100;
            PrimName = "Tri Sprites";
        } else if(PrimType == 0x0100) {
            PrimType = 0x0101;
            PrimName = "Quad Sprites";
        } else if(PrimType == 0x0101) {
            PrimType = 0x0103;
            PrimName = "Display List";
        } else if(PrimType == 0x0103) {
            PrimType = 0x0104;
            PrimName = "Not Drawing";
        } else if(PrimType == 0x0104) {
            PrimType = GL_POINTS;
            PrimName = "Points";
        }
        break;
    case 't':
        ShowText = !ShowText;
        break;
    case 'd':
        DepthTest = !DepthTest;
        if(DepthTest)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
        cerr << "DepthTest " << (DepthTest ? "on":"off") << ".\n";
        break;
    case 'g':
        DrawGround = !DrawGround;
        break;
    case 's':
        SortParticles = !SortParticles;
        break;
    case 'w':
        ParticleCam = !ParticleCam;
        break;
    case 'z':
        P.SinkVelocity(true, PDSphere(pVec(0, 0, 0), 0.01));
        break;
    case 'x':
        FreezeParticles = !FreezeParticles;
        break;
    case '=':
    case '+':
        Efx.maxParticles += 1000;
        P.SetMaxParticles(Efx.maxParticles);
        cerr << Efx.maxParticles << endl;
        break;
    case '-':
    case '_':
        Efx.maxParticles -= 1000;
        if(Efx.maxParticles<0) Efx.maxParticles = 0;
        P.SetMaxParticles(Efx.maxParticles);
        cerr << Efx.maxParticles << endl;
        break;
    case '>':
        BlurRate -= 0.01;
        if(BlurRate < 0)
            BlurRate = 0;
        break;
    case '<':
        BlurRate += 0.01;
        if(BlurRate >= 1)
            BlurRate = 1;
        break;
    case 'q':
    case '\033':   /* ESC key: quit */
        exit(0);
        break;
    }

    if(item > '0' && item <= '9') {
        Efx.numSteps = item - '0';
        P.TimeStep(1 / float(Efx.numSteps));
    }

    glutPostRedisplay();
}

void SKeyPress(int key, int x, int y)
{
    menu((int) key + 0x1000);
}

void KeyPress(unsigned char key, int x, int y)
{
    menu((int) key);
}

static void Usage(char *program_name, char *message)
{
    if (message)
        cerr << message << endl;

    cerr << "Usage: " << program_name << endl;
    exit(1);
}

static void Args(int argc, char **argv)
{
    char *program = argv[0];

    for(int i=1; i<argc; i++) {
        if(string(argv[i]) == "-h" || string(argv[i]) == "-help") {
            Usage(program, "Help:");
        } else if(string(argv[i]) == "-photo") {
            FName = argv[i+1];
            Efx.SetPhoto(new uc3Image(FName));

            RemoveArgs(argc, argv, i, 2);
        } else if(string(argv[i]) == "-bench") {
            SetupBenchmark();
            RemoveArgs(argc, argv, i);
        } else if(string(argv[i]) == "-spot") {
            SphereTexture = false;
            RemoveArgs(argc, argv, i);
        } else {
            Usage(program, "Invalid option!");
        }
    }
}

void GlutSetup(int argc, char **argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutInitWindowSize(900, 900);
    glutInitWindowPosition(200, 200);
    glutCreateWindow("Particle Spray");

    glutDisplayFunc(Draw);
    glutIdleFunc(Draw);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(KeyPress);
    glutSpecialFunc(SKeyPress);

    glutCreateMenu(menu);
    glutAddMenuEntry("1: 1 step per frame", '1');
    glutAddMenuEntry("2: 2 steps per frame", '2');
    glutAddMenuEntry("3: 3 steps ...", '3');
    glutAddMenuEntry("space : Explosion", ' ');
    glutAddMenuEntry("g: Draw ground", 'g');
    glutAddMenuEntry("a: Toggle antialiasing", 'a');
    glutAddMenuEntry("p: Cycle Primitive", 'p');
    glutAddMenuEntry("t: Show Text", 't');
    glutAddMenuEntry("r: Toggle random demo", 'r');
    glutAddMenuEntry("w: ParticleCam", 'w');
    glutAddMenuEntry("s: Sort particles", 's');
    glutAddMenuEntry("m: Toggle motion blur", 'm');
    glutAddMenuEntry("c: Toggle camera motion", 'c');
    glutAddMenuEntry("d: Toggle depth test", 'd');
    glutAddMenuEntry("i: Toggle immed mode", 'i');
    glutAddMenuEntry("+: 1000 more particles", '+');
    glutAddMenuEntry("-: 1000 less particles", '-');
    glutAddMenuEntry(">: More motion blur", '>');
    glutAddMenuEntry("<: Less motion blur", '<');
    glutAddMenuEntry("x: Freeze Particles", 'x');

    glutAddMenuEntry("<esc> or q: exit program", '\033');
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char **argv)
{
    srand48( (unsigned)time( NULL ) );

    DemoNum = lrand48();
    RandomDemoClock.Start();

    if(!HasSSE()) {
        cerr << "Your CPU must have SSE to run this program. You should get an Intel or AMD CPU or recompile with SSE turned off.\n";
        throw;
    }

    Args(argc, argv);
    GlutSetup(argc, argv);

    // Make a particle group
    Efx.particle_handle = P.GenParticleGroups(1, Efx.maxParticles);

    P.CurrentGroup(Efx.particle_handle);

    Efx.CallDemo(DemoNum, true, Immediate);

    try {
        glutMainLoop();
    }
    catch (PError_t &Er) {
        cerr << "Particle API exception: " << Er.ErrMsg << endl;
        throw Er;
    }
    catch (...) {
        cerr << "Non-Particle-API exception caught. Bye.\n";
        throw;
    }

    return 0;
}
