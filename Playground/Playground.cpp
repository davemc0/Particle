// Playground.cpp
//
/// Copyright 1997-2007, 2022 by David K. McAllister
//
// This application demonstrates particle systems for interactive graphics.
// It uses OpenGL and GLUT.

#include "../DemoShared/DrawGroups.h"
#include "../DemoShared/Effects.h"
#include "../DemoShared/Monarch.h"
#include "Particle/pAPI.h"

// The following header files are part of DMcTools.
// DMcTools is part of the same source distribution as the Particle API.
#include "Math/Random.h"
#include "Util/Assert.h"
#include "Util/StatTimer.h"
#include "Util/Timer.h"

// OpenGL
#include "GL/glew.h"

// This needs to come after GLEW
#include "GL/freeglut.h"

#include <cmath>
#include <cstring>
#include <iostream>
#include <string>

namespace {
// Mode controls
bool CameraMotion = false;
bool DrawEffectGeometry = true;
bool FreezeParticles = false;
bool FullScreen = false;
bool ParticleCam = false;
bool RandomDemo = true;
bool ShowText = true;
bool WantEffectSettings = false;

// Rendering params controlled by the effect
PrimType_e PrimType = PRIM_SPHERE_SPRITE;
bool DepthTest = false;
bool MotionBlur = false;
bool SortParticles = false;

// Internal state
ExecMode_e ExecMode = Internal_Mode; // Execute compiled action lists on host for non-varying effects or immediate for varying
ParticleContext_t P;
StatTimer FPSClock(100);
Timer RandomDemoClock;
char* FName = NULL;
float BlurRate = 0.09;
int DisplayListID = -1;
int SpotTexID = -1, SphereTexID = -1;
int WinWidth = 880, WinHeight = 880;
#ifdef _DEBUG
EffectsManager Efx(P, 20000);
#else
EffectsManager Efx(P, 200000);
#endif
} // namespace

// Render any geometry necessary to support the effects
void RenderEffectGeometry()
{
    for (const auto& domPtr : Efx.Demo->Renderables) {
        pVec v;
        glColor3ub(0, 115, 0);
        if (dynamic_cast<PDSphere*>(domPtr.get())) {
            PDSphere* dom = dynamic_cast<PDSphere*>(domPtr.get());
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glTranslatef(dom->ctr.x(), dom->ctr.y(), dom->ctr.z());
            glutWireSphere(dom->radOut, 32, 16);
            glPopMatrix();
        } else if (dynamic_cast<PDBox*>(domPtr.get())) {
            PDBox* dom = dynamic_cast<PDBox*>(domPtr.get());
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            pVec ctr = (dom->p1 + dom->p0) * 0.5f, extent = dom->p1 - dom->p0;
            glTranslatef(ctr.x(), ctr.y(), ctr.z());
            glScalef(extent.x(), extent.y(), extent.z());
            glutWireCube(1);
            glPopMatrix();
        } else if (dynamic_cast<PDDisc*>(domPtr.get())) {
            PDDisc* dom = dynamic_cast<PDDisc*>(domPtr.get());
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            // TODO: Need to rotate orientation of disc
            glTranslatef(dom->p.x(), dom->p.y(), dom->p.z());
            GLUquadric* Q = gluNewQuadric();
            gluDisk(Q, dom->radIn, dom->radOut, 64, 4);
            glPopMatrix();
        } else if (dynamic_cast<PDRectangle*>(domPtr.get())) {
            PDRectangle* dom = dynamic_cast<PDRectangle*>(domPtr.get());
            glColor3ub(0, 100, 40);
            glBegin(GL_QUADS);
            glVertex3fv((GLfloat*)&(v = dom->p));
            glVertex3fv((GLfloat*)&(v = dom->p + dom->u));
            glVertex3fv((GLfloat*)&(v = dom->p + dom->u + dom->v));
            glVertex3fv((GLfloat*)&(v = dom->p + dom->v));
            glEnd();
        } else if (dynamic_cast<PDTriangle*>(domPtr.get())) {
            PDTriangle* dom = dynamic_cast<PDTriangle*>(domPtr.get());
            glBegin(GL_TRIANGLES);
            glVertex3fv((GLfloat*)&(v = dom->p));
            glVertex3fv((GLfloat*)&(v = dom->p + dom->u));
            glVertex3fv((GLfloat*)&(v = dom->p + dom->v));
            glEnd();
        } else if (dynamic_cast<PDPlane*>(domPtr.get())) {
            PDPlane* dom = dynamic_cast<PDPlane*>(domPtr.get());
            const float planeDist = 100.f;
            pVec uu = Cross(pVec(1, 0, 0), dom->nrm);
            pVec vv = Cross(dom->nrm, uu);
            uu *= planeDist;
            vv *= planeDist;
            glBegin(GL_TRIANGLE_FAN);
            glColor3f(0.6, 0.7, 0.8);
            glVertex3fv((GLfloat*)&(v = dom->p));
            glColor3f(1.0, 1.0, 1.0);
            glVertex3fv((GLfloat*)&(v = dom->p - uu - vv));
            glColor3f(1.0, 1.0, 1.0);
            glVertex3fv((GLfloat*)&(v = dom->p + uu - vv));
            glColor3f(1.0, 1.0, 1.0);
            glVertex3fv((GLfloat*)&(v = dom->p + uu + vv));
            glColor3f(1.0, 1.0, 1.0);
            glVertex3fv((GLfloat*)&(v = dom->p - uu + vv));
            glColor3f(1.0, 1.0, 1.0);
            glVertex3fv((GLfloat*)&(v = dom->p - uu - vv));
            glEnd();
        } else if (dynamic_cast<PDLine*>(domPtr.get())) {
            PDLine* dom = dynamic_cast<PDLine*>(domPtr.get());
            glBegin(GL_LINES);
            glVertex3fv((GLfloat*)&(v = dom->p0));
            glVertex3fv((GLfloat*)&(v = dom->p1));
            glEnd();
        } else if (dynamic_cast<PDPoint*>(domPtr.get())) {
            PDPoint* dom = dynamic_cast<PDPoint*>(domPtr.get());
            glBegin(GL_POINTS);
            glVertex3fv((GLfloat*)&(v = dom->p));
            glEnd();
        } else {
            EASSERT(0);
        }
    }
}

// Call this after choosing an effect to set the render settings to those of this effect
void ApplyEffectSettings()
{
    if (!WantEffectSettings) return;

    if (!Efx.Demo->getUseRenderingParams()) return; // The effect does not want to give us any rendering params.

    PrimType = Efx.Demo->getPrimType();
    DepthTest = Efx.Demo->getDepthTest();
    MotionBlur = Efx.Demo->getMotionBlur();
    SortParticles = Efx.Demo->getSortParticles();
}

// Symmetric gaussian centered at origin; no covariance matrix
inline float Gaussian2(float x, float y, float sigma) { return expf(-0.5f * (x * x + y * y) / (sigma * sigma)) / (P_SQRT2PI * sigma); }

void MakeGaussianSpotTexture()
{
    const int DIM = 64;
    const int DIM2 = DIM / 2;
    const float TEX_SCALE = 6.0;

    glGenTextures(1, (GLuint*)&SpotTexID);
    glBindTexture(GL_TEXTURE_2D, SpotTexID);

    std::shared_ptr<float[]> img(new float[DIM * DIM]);

    for (int y = 0; y < DIM; y++) {
        for (int x = 0; x < DIM; x++) {
            // Clamping the edges to zero allows Nvidia's blend optimizations to do their thing.
            if (x < 2 || x >= DIM - 2 || y < 2 || y >= DIM - 2)
                img[y * DIM + x] = 0;
            else {
                img[y * DIM + x] = TEX_SCALE * Gaussian2(x - DIM2, y - DIM2, (DIM * 0.15));
            }
        }
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    GL_ASSERT();

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA16, DIM, DIM, GL_ALPHA, GL_FLOAT, img.get());
}

void MakeSphereTexture()
{
    const int DIM = 512;
    const int CTR = DIM / 2;
    const int RAD = CTR - 16; // Clamping the edges to zero allows Nvidia's blend optimizations to do their thing.

    glGenTextures(1, (GLuint*)&SphereTexID);
    glBindTexture(GL_TEXTURE_2D, SphereTexID);

    std::shared_ptr<float[]> img(new float[DIM * DIM]);

    pVec light = pVec(1, 1, 3);
    light.normalize();

    for (int y = 0; y < DIM; y++) {
        for (int x = 0; x < DIM; x++) {
            pVec p = pVec(x, y, 0) - pVec(CTR, CTR, 0);
            float len = p.length();
            float z = sqrt(RAD * RAD - len * len);
            p.z() = z;
            if (len >= RAD) {
                img[y * DIM + x] = 0.0;
            } else {
                p.normalize();
                float v = dot(p, light);
                if (v < 0) v = 0;
                img[y * DIM + x] = v;
            }
        }
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    GL_ASSERT();

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA16, DIM, DIM, GL_ALPHA, GL_FLOAT, img.get());
}

static void showBitmapMessage(GLfloat x, GLfloat y, char* message)
{
    if (message == NULL) return;

    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WinWidth, WinHeight, 0, -1, 1);
    glRasterPos3f(x, y, -1.f);

    while (*message) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message);
        message++;
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPopAttrib();
}

void InitOpenGL()
{
    static bool didInit = false;
    if (didInit) return;
    didInit = true;

    if (glewInit() != GLEW_OK) throw PError_t("No GLEW");

    // Make the point size attenuate with distance.
    // These numbers are arbitrary and need to be fixed for accuracy.
    // The most correct way to do this is to compute the determinant of the upper 3x3 of the
    // ModelView + Viewport matrix. This gives a measure of the change in size from model space
    // to eye space. The cube root of this estimates the 1D change in scale. Divide this by W per point.
    float params[3] = {0.0f, 0.0f, 0.00003f};
    glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, params);
    glPointParameterf(GL_POINT_SIZE_MIN, 0);
    glPointParameterf(GL_POINT_SIZE_MAX, 5000);
    glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, 1);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glLineWidth(1.0);
    glPointSize(1.0);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 2 / 255.0);

    if (DepthTest)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(1.0, 1.0, 1.0, 0.0);

    // Texture unit state
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    float col[4] = {1.f, 1.f, 1.f, 1.f};
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, col);
    glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

    MakeSphereTexture();
    MakeGaussianSpotTexture();

    DisplayListID = glGenLists(1);
    if (1)
        Monarch(DisplayListID);
    else {
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
    }
}

void Draw()
{
    InitOpenGL();

    glLoadIdentity();

    if (false && MotionBlur) {
        // This is a cheezy motion blur that dims the old frame contents
        // before rendering the new ones. Requires single-buffering.
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

        glDrawBuffer(GL_FRONT);
        glColor4f(0, 0, 0, BlurRate);
        glRectf(-1, -1, 1, 1);
        if (DepthTest) glClear(GL_DEPTH_BUFFER_BIT);

        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // XXX: When do we restore the projection matrix for normal rendering?
    } else {
        glClear(GL_COLOR_BUFFER_BIT | (DepthTest ? GL_DEPTH_BUFFER_BIT : 0));
    }

    // Use a particle to model the camera pose
    float CamSpeed = 3.f;
    static int CameraSystem = -1;
    if (CameraSystem < 0) {
        CameraSystem = P.GenParticleGroups(1, 1);
        P.CurrentGroup(CameraSystem);

        pSourceState S;
        S.Velocity(PDSphere(pVec(0, 0, 0), CamSpeed, CamSpeed));
        P.Vertex(pVec(0, -24, Efx.center.z()), S);
    }

    P.CurrentGroup(CameraSystem);
    if (CameraMotion) {
        P.Bounce(0, 1, 0, PDBox(pVec(-15, -4, 1), pVec(15, -30, 12)));
        P.SpeedLimit(CamSpeed, CamSpeed);
        P.Move();
    }

    // Make the camera track a particle
    if (ParticleCam) P.CurrentGroup(Efx.particleHandle);

    pVec Cam, Vel;
    P.GetParticles(0, 1, (float*)&Cam, NULL, (float*)&Vel);

#if 0
    pVec At=Cam+Vel; // Look in the direction the camera is flying
#else
    pVec At = Efx.center; // Look at the center of action
#endif
    gluLookAt(Cam.x(), Cam.y(), Cam.z(), At.x(), At.y(), At.z(), 0, 0, 1);

    if (DrawEffectGeometry) RenderEffectGeometry();

    // Do the particle dynamics
    if (!FreezeParticles) {
        P.CurrentGroup(Efx.particleHandle);
        for (int step = 0; step < Efx.simStepsPerFrame; step++) { Efx.RunDemoFrame(ExecMode); }
    }

    P.CurrentGroup(Efx.particleHandle);
    if (SortParticles) {
        pVec Look = DepthTest ? (At - Cam) : (Cam - At);
        P.Sort(Cam, Look);
    }

    // Draw the particle system
    pVec view = At - Cam;
    view.normalize();
    pVec up = pVec(0, 0, 1);

    switch (PrimType) {
    case PRIM_DISPLAY_LIST:
        DrawGroupAsDisplayLists(P, DisplayListID, false, false);
        // The cross product of the velocity vector and the previous frame's velocity vector
        // can give us a vector pointing to the side. Use this to orient the particles.
        P.CopyVertexB(false, true);
        break;
    case PRIM_LINE:
        // DrawGroupAsLines(P, false, Efx.timeStep);
        DrawGroupAsLines(P, false, -0.25f);
        break;
    case PRIM_POINT:
        glDisable(GL_POINT_SPRITE);
        DrawGroupAsPoints(P, false);
        break;
    case PRIM_GAUSSIAN_SPRITE:
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_POINT_SPRITE);
        glBindTexture(GL_TEXTURE_2D, SpotTexID);
        DrawGroupAsPoints(P, false);
        glDisable(GL_POINT_SPRITE);
        glDisable(GL_TEXTURE_2D);
        break;
    case PRIM_SPHERE_SPRITE:
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_POINT_SPRITE);
        glBindTexture(GL_TEXTURE_2D, SphereTexID);
        DrawGroupAsPoints(P, false);
        glDisable(GL_POINT_SPRITE);
        glDisable(GL_TEXTURE_2D);
        break;
    case PRIM_QUAD:
        glEnable(GL_TEXTURE_2D);
        DrawGroupAsQuadSprites(P, view, up, 1.f, true, false, false);
        glDisable(GL_TEXTURE_2D);
        break;
    case PRIM_NONE:
        // Don't draw.
        break;
    default: EASSERT(0);
    }

    GL_ASSERT();

    float frameTime = FPSClock.Event();

    // Adjust simulation time step based on frame rate
    Efx.timeStep = frameTime / float(Efx.simStepsPerFrame);
    P.TimeStep(Efx.timeStep);

    // Draw the text.
    if (ShowText) {
        glLoadIdentity();
        char msg[256];
        float fps = 1.f / FPSClock.GetMean();

        int cnt = (int)P.GetGroupCount();
        char exCh = (ExecMode == Immediate_Mode) ? 'I' : (ExecMode == Internal_Mode) ? 'N' : 'C';

        sprintf(msg, " %c%c%c%c%c%c%c%c n=%5d iters=%d fps=%02.2f %s %s t=%1.2f", MotionBlur ? 'B' : ' ', FreezeParticles ? 'F' : ' ',
                WantEffectSettings ? 'A' : ' ', RandomDemo ? 'R' : ' ', DepthTest ? 'D' : ' ', exCh, CameraMotion ? 'M' : ' ', SortParticles ? 'S' : ' ', cnt,
                Efx.simStepsPerFrame, fps, PrimTypeNames[PrimType], Efx.GetCurEffectName().c_str(), RandomDemoClock.Read());

        glColor3f(1, 1, 1);
        showBitmapMessage(100, 50, msg);
        glColor3f(0, 0, 0);
        showBitmapMessage(99, 49, msg);
    }

    if (!MotionBlur) glutSwapBuffers();

    if (RandomDemo && RandomDemoClock.Read() > Efx.demoRunSec) {
        RandomDemoClock.Reset();
        Efx.ChooseDemo(-2, ExecMode); // Change to a different random demo
        ApplyEffectSettings();
    }
}

void Reshape(int w, int h)
{
    WinWidth = w;
    WinHeight = h;

    glViewport(0, 0, WinWidth, WinHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40, WinWidth / double(WinHeight), 1, 100);
    glMatrixMode(GL_MODELVIEW);

    // Useful for motion blur so background doesn't get ugly.
    glClear(GL_COLOR_BUFFER_BIT | (DepthTest ? GL_DEPTH_BUFFER_BIT : 0));
}

void menu(int item)
{
    static int OldWidth, OldHeight, OldX, OldY;

    switch (item) {
    case ' ':
        RandomDemoClock.Reset();
        Efx.ChooseDemo(3, ExecMode); // Explosion
        ApplyEffectSettings();
        break;
    case GLUT_KEY_UP + 0x1000:
        RandomDemoClock.Reset();
        Efx.ChooseDemo(13, ExecMode); // Restore
        ApplyEffectSettings();
        break;
    case GLUT_KEY_DOWN + 0x1000:
        RandomDemoClock.Reset();
        Efx.ChooseDemo(-2, ExecMode); // Random
        ApplyEffectSettings();
        break;
    case GLUT_KEY_LEFT + 0x1000:
        RandomDemoClock.Reset();
        Efx.ChooseDemo(Efx.demoNum - 1, ExecMode); // Prev
        ApplyEffectSettings();
        break;
    case GLUT_KEY_RIGHT + 0x1000:
        RandomDemoClock.Reset();
        Efx.ChooseDemo(Efx.demoNum + 1, ExecMode); // Next
        ApplyEffectSettings();
        break;
    case 'i':
        if (ExecMode == Immediate_Mode)
            ExecMode = Internal_Mode;
        else if (ExecMode == Internal_Mode)
            ExecMode = Compiled_Mode;
        else
            ExecMode = Immediate_Mode;

        std::cerr << "Switching to " << ((ExecMode == Immediate_Mode) ? "Immediate" : (ExecMode == Internal_Mode) ? "Internal" : "Compiled") << " mode.\n";
        break;
    case 'm':
        MotionBlur = !MotionBlur;
        if (!MotionBlur) glDrawBuffer(GL_BACK);
        break;
    case 'a': WantEffectSettings = !WantEffectSettings; break;
    case 'c': CameraMotion = !CameraMotion; break;
    case 'r': RandomDemo = !RandomDemo; break;
    case 'f':
        FullScreen = !FullScreen;
        if (FullScreen) {
            OldWidth = glutGet(GLenum(GLUT_WINDOW_WIDTH));
            OldHeight = glutGet(GLenum(GLUT_WINDOW_HEIGHT));
            OldX = glutGet(GLenum(GLUT_WINDOW_X));
            OldY = glutGet(GLenum(GLUT_WINDOW_Y));
            glutSetCursor(GLUT_CURSOR_NONE);
            glutFullScreen();
        } else {
            glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
            glutReshapeWindow(OldWidth, OldHeight);
            glutPositionWindow(OldX, OldY);
        }
        break;
    case 'p': PrimType = (PrimType_e)((PrimType + 1) % PRIM_TYPE_COUNT); break;
    case 't': ShowText = !ShowText; break;
    case 'd':
        DepthTest = !DepthTest;
        if (DepthTest)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
        std::cerr << "DepthTest " << (DepthTest ? "on" : "off") << ".\n";
        break;
    case 'g': DrawEffectGeometry = !DrawEffectGeometry; break;
    case 's': SortParticles = !SortParticles; break;
    case 'w': ParticleCam = !ParticleCam; break;
    case 'z': P.SinkVelocity(true, PDSphere(pVec(0, 0, 0), 0.01)); break;
    case 'x': FreezeParticles = !FreezeParticles; break;
    case '=':
    case '+':
        Efx.maxParticles += 5000;
        P.SetMaxParticles(Efx.maxParticles);
        std::cerr << Efx.maxParticles << std::endl;
        break;
    case '-':
    case '_':
        Efx.maxParticles -= 5000;
        if (Efx.maxParticles < 0) Efx.maxParticles = 0;
        P.SetMaxParticles(Efx.maxParticles);
        std::cerr << Efx.maxParticles << std::endl;
        break;
    case '>':
        BlurRate -= 0.01;
        if (BlurRate < 0) BlurRate = 0;
        break;
    case '<':
        BlurRate += 0.01;
        if (BlurRate >= 1) BlurRate = 1;
        break;
    case 'q':
    case '\033': /* ESC key: quit */ exit(0); break;
    }

    if (item > '0' && item <= '9') { Efx.simStepsPerFrame = item - '0'; }

    glutPostRedisplay();
}

void SKeyPress(int key, int x, int y) { menu((int)key + 0x1000); }

void KeyPress(unsigned char key, int x, int y) { menu((int)key); }

static void Usage(char* program_name, char* message)
{
    if (message) std::cerr << message << std::endl;

    std::cerr << "Usage: " << program_name << std::endl;
    exit(1);
}

static void Args(int argc, char** argv)
{
    char* program = argv[0];

    for (int i = 1; i < argc; i++) {
        std::string starg(argv[i]);
        if (starg == "-h" || starg == "-help") {
            Usage(program, "Help:");
        } else if (starg == "-photo") {
            FName = argv[i + 1];
            Efx.SetPhoto(new uc3Image(FName));

            RemoveArgs(argc, argv, i, 2);
        } else {
            Usage(program, "Invalid option!");
        }
    }
}

void GlutSetup(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutInitWindowSize(WinWidth, WinHeight);
    glutInitWindowPosition(300, 0);
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
    glutAddMenuEntry("x: Freeze Particles", 'x');
    glutAddMenuEntry("r: Toggle random demo", 'r');
    glutAddMenuEntry("w: ParticleCam", 'w');
    glutAddMenuEntry("+: 5000 more particles", '+');
    glutAddMenuEntry("-: 5000 less particles", '-');

    glutAddMenuEntry("p: Cycle Primitive", 'p');
    glutAddMenuEntry("g: Toggle draw effect geometry", 'g');
    glutAddMenuEntry("a: Toggle apply effect settings", 'a');
    glutAddMenuEntry("s: Toggle sort particles", 's');
    glutAddMenuEntry("m: Toggle motion blur", 'm');
    glutAddMenuEntry("c: Toggle camera motion", 'c');
    glutAddMenuEntry("d: Toggle depth test", 'd');
    glutAddMenuEntry("i: Toggle immediate mode", 'i');
    glutAddMenuEntry(">: More motion blur", '>');
    glutAddMenuEntry("<: Less motion blur", '<');
    glutAddMenuEntry("t: Show Text", 't');

    glutAddMenuEntry("<esc> or q: exit program", '\033');
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char** argv)
{
    SRand();

    Args(argc, argv);
    GlutSetup(argc, argv);

    // Make a particle group
    Efx.particleHandle = P.GenParticleGroups(1, Efx.maxParticles);

    P.CurrentGroup(Efx.particleHandle);

    Efx.MakeActionLists(ExecMode);

    Efx.ChooseDemo(-2, ExecMode); // Random
    ApplyEffectSettings();

    try {
        glutMainLoop();
    }
    catch (PError_t& Er) {
        std::cerr << "Particle API exception: " << Er.ErrMsg << std::endl;
        throw Er;
    }
    catch (...) {
        std::cerr << "Non-Particle-API exception caught. Bye.\n";
        throw;
    }

    return 0;
}
