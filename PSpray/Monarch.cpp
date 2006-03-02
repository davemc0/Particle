
#ifdef WIN32
// This is for something in gl.h.
#include <windows.h>
#endif

#include <GL/gl.h>

// Make a Monarch Butterfly by hand.
// In about 1993 I drew this butterfly on graph paper and typed in the coords.
// I can't believe I ever did that.
void Monarch(int ListNum)
{
	glNewList(ListNum, GL_COMPILE);

	glScaled(4.0, 4.0, 4.0);
	glRotated(-90.0, 0, 0, 1);

	// Left Wing
	glBegin(GL_TRIANGLE_FAN);

	glNormal3d(0.0, 0.0, 1.0);
	glVertex3d(-0.075, 0.0, 0.0);

	glNormal3d(0.707, 0.707, 0.000);
	glVertex3d(-0.015, 0.0, 0.0);
	glNormal3d(0.659, 0.707, 0.255);
	glVertex3d(-0.019, 0.030, 0.0);
	glNormal3d(0.523, 0.707, 0.476);
	glVertex3d(-0.015, 0.060, 0.0);
	glNormal3d(0.315, 0.707, 0.633);
	glVertex3d(-0.045, 0.069, 0.0);
	glNormal3d(0.065, 0.707, 0.704);
	glVertex3d(-0.075, 0.075, 0.0);
	glNormal3d(-0.194, 0.707, 0.680);
	glVertex3d(-0.105, 0.069, 0.0);
	glNormal3d(-0.426, 0.707, 0.564);
	glVertex3d(-0.135, 0.060, 0.0);
	glNormal3d(-0.601, 0.707, 0.372);
	glVertex3d(-0.150, 0.030, 0.0);
	glNormal3d(-0.695, 0.707, 0.130);
	glVertex3d(-0.141, 0.0, 0.0);
	glNormal3d(-0.695, 0.707, -0.130);
	glVertex3d(-0.126, -0.030, 0.0);
	glNormal3d(-0.601, 0.707, -0.372);
	glVertex3d(-0.108, -0.060, 0.0);
	glNormal3d(-0.426, 0.707, -0.564);
	glVertex3d(-0.075, -0.084, 0.0);
	glNormal3d(-0.194, 0.707, -0.680);
	glVertex3d(-0.045, -0.129, 0.0);
	glNormal3d(0.065, 0.707, -0.704);
	glVertex3d(-0.030, -0.120, 0.0);
	glNormal3d(0.315, 0.707, -0.633);
	glVertex3d(-0.009, -0.105, 0.0);
	glNormal3d(0.523, 0.707, -0.476);
	glVertex3d(-0.015, -0.060, 0.0);
	glNormal3d(0.659, 0.707, -0.255);
	glVertex3d(-0.015, -0.030, 0.0);
	glNormal3d(0.707, 0.707, -0.000);
	glVertex3d(-0.015, -0.0, 0.0);

	glEnd();

	// Right Wing
	glBegin(GL_TRIANGLE_FAN);

	glNormal3d(0.0, 0.0, 1.0);
	glVertex3d(0.075, 0.0, 0.0);

	glNormal3d(0.707, 0.707, 0.000);
	glVertex3d(0.015, -0.0, 0.0);
	glNormal3d(0.659, 0.707, 0.255);
	glVertex3d(0.015, -0.030, 0.0);
	glNormal3d(0.523, 0.707, 0.476);
	glVertex3d(0.015, -0.060, 0.0);
	glNormal3d(0.315, 0.707, 0.633);
	glVertex3d(0.009, -0.105, 0.0);
	glNormal3d(0.065, 0.707, 0.704);
	glVertex3d(0.030, -0.120, 0.0);
	glNormal3d(-0.194, 0.707, 0.680);
	glVertex3d(0.045, -0.129, 0.0);
	glNormal3d(-0.426, 0.707, 0.564);
	glVertex3d(0.075, -0.084, 0.0);
	glNormal3d(-0.601, 0.707, 0.372);
	glVertex3d(0.108, -0.060, 0.0);
	glNormal3d(-0.695, 0.707, 0.130);
	glVertex3d(0.126, -0.030, 0.0);
	glNormal3d(-0.695, 0.707, -0.130);
	glVertex3d(0.141, 0.0, 0.0);
	glNormal3d(-0.601, 0.707, -0.372);
	glVertex3d(0.150, 0.030, 0.0);
	glNormal3d(-0.426, 0.707, -0.564);
	glVertex3d(0.135, 0.060, 0.0);
	glNormal3d(-0.194, 0.707, -0.680);
	glVertex3d(0.105, 0.069, 0.0);
	glNormal3d(0.065, 0.707, -0.704);
	glVertex3d(0.075, 0.075, 0.0);
	glNormal3d(0.315, 0.707, -0.633);
	glVertex3d(0.045, 0.069, 0.0);
	glNormal3d(0.523, 0.707, -0.476);
	glVertex3d(0.015, 0.060, 0.0);
	glNormal3d(0.659, 0.707, -0.255);
	glVertex3d(0.019, 0.030, 0.0);
	glNormal3d(0.707, 0.707, -0.000);
	glVertex3d(0.015, 0.0, 0.0);

	glEnd();

	glColor3d(.5, .5, 1);

	// Body contour
	glBegin(GL_QUAD_STRIP);
	glNormal3d(1.000, 0.000, 0.000);
	glVertex3d(0.000, 0.081, 0.000);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.000, 0.081, 0.000);
	glNormal3d(1.000, 0.000, 0.000);
	glVertex3d(0.007, 0.078, 0.000);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.005, 0.078, 0.005);
	glNormal3d(1.000, 0.000, 0.000);
	glVertex3d(0.012, 0.072, 0.000);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.008, 0.072, 0.008);
	glNormal3d(1.000, 0.000, 0.000);
	glVertex3d(0.015, 0.060, 0.000);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.011, 0.060, 0.011);
	glNormal3d(1.000, 0.000, 0.000);
	glVertex3d(0.019, 0.030, 0.000);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.014, 0.030, 0.014);
	glNormal3d(1.000, 0.000, 0.000);
	glVertex3d(0.021, 0.000, 0.000);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.015, 0.000, 0.015);
	glNormal3d(1.000, 0.000, 0.000);
	glVertex3d(0.021, -0.030, 0.000);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.015, -0.030, 0.015);
	glNormal3d(1.000, 0.000, 0.000);
	glVertex3d(0.015, -0.060, 0.000);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.011, -0.060, 0.011);
	glNormal3d(1.000, 0.000, 0.000);
	glVertex3d(0.009, -0.105, 0.000);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.006, -0.105, 0.006);
	glNormal3d(1.000, 0.000, 0.000);
	glVertex3d(0.000, -0.120, 0.000);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.000, -0.120, 0.000);
	glEnd();

	glBegin(GL_QUAD_STRIP);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.000, 0.081, 0.000);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, 0.081, 0.000);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.005, 0.078, 0.005);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, 0.078, 0.007);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.008, 0.072, 0.008);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, 0.072, 0.012);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.011, 0.060, 0.011);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, 0.060, 0.015);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.014, 0.030, 0.014);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, 0.030, 0.019);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.015, 0.000, 0.015);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, 0.000, 0.021);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.015, -0.030, 0.015);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, -0.030, 0.021);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.011, -0.060, 0.011);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, -0.060, 0.015);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.006, -0.105, 0.006);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, -0.105, 0.009);
	glNormal3d(0.707, 0.000, 0.707);
	glVertex3d(0.000, -0.120, 0.000);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, -0.120, 0.000);
	glEnd();

	glBegin(GL_QUAD_STRIP);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, 0.081, 0.000);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.000, 0.081, 0.000);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, 0.078, 0.007);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.005, 0.078, 0.005);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, 0.072, 0.012);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.008, 0.072, 0.008);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, 0.060, 0.015);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.011, 0.060, 0.011);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, 0.030, 0.019);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.014, 0.030, 0.014);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, 0.000, 0.021);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.015, 0.000, 0.015);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, -0.030, 0.021);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.015, -0.030, 0.015);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, -0.060, 0.015);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.011, -0.060, 0.011);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, -0.105, 0.009);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.006, -0.105, 0.006);
	glNormal3d(0.000, 0.000, 1.000);
	glVertex3d(0.000, -0.120, 0.000);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.000, -0.120, 0.000);
	glEnd();

	glBegin(GL_QUAD_STRIP);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.000, 0.081, 0.000);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.000, 0.081, 0.000);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.005, 0.078, 0.005);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.007, 0.078, 0.000);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.008, 0.072, 0.008);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.012, 0.072, 0.000);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.011, 0.060, 0.011);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.015, 0.060, 0.000);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.014, 0.030, 0.014);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.019, 0.030, 0.000);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.015, 0.000, 0.015);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.021, 0.000, 0.000);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.015, -0.030, 0.015);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.021, -0.030, 0.000);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.011, -0.060, 0.011);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.015, -0.060, 0.000);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.006, -0.105, 0.006);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.009, -0.105, 0.000);
	glNormal3d(-0.707, 0.000, 0.707);
	glVertex3d(-0.000, -0.120, 0.000);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.000, -0.120, 0.000);
	glEnd();

	glBegin(GL_QUAD_STRIP);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.000, 0.081, 0.000);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.000, 0.081, -0.000);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.007, 0.078, 0.000);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.005, 0.078, -0.005);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.012, 0.072, 0.000);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.008, 0.072, -0.008);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.015, 0.060, 0.000);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.011, 0.060, -0.011);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.019, 0.030, 0.000);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.014, 0.030, -0.014);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.021, 0.000, 0.000);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.015, 0.000, -0.015);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.021, -0.030, 0.000);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.015, -0.030, -0.015);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.015, -0.060, 0.000);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.011, -0.060, -0.011);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.009, -0.105, 0.000);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.006, -0.105, -0.006);
	glNormal3d(-1.000, 0.000, 0.000);
	glVertex3d(-0.000, -0.120, 0.000);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.000, -0.120, -0.000);
	glEnd();

	glBegin(GL_QUAD_STRIP);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.000, 0.081, -0.000);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, 0.081, -0.000);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.005, 0.078, -0.005);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, 0.078, -0.007);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.008, 0.072, -0.008);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, 0.072, -0.012);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.011, 0.060, -0.011);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, 0.060, -0.015);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.014, 0.030, -0.014);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, 0.030, -0.019);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.015, 0.000, -0.015);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, 0.000, -0.021);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.015, -0.030, -0.015);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, -0.030, -0.021);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.011, -0.060, -0.011);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, -0.060, -0.015);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.006, -0.105, -0.006);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, -0.105, -0.009);
	glNormal3d(-0.707, 0.000, -0.707);
	glVertex3d(-0.000, -0.120, -0.000);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, -0.120, -0.000);
	glEnd();

	glBegin(GL_QUAD_STRIP);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, 0.081, -0.000);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.000, 0.081, -0.000);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, 0.078, -0.007);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.005, 0.078, -0.005);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, 0.072, -0.012);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.008, 0.072, -0.008);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, 0.060, -0.015);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.011, 0.060, -0.011);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, 0.030, -0.019);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.014, 0.030, -0.014);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, 0.000, -0.021);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.015, 0.000, -0.015);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, -0.030, -0.021);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.015, -0.030, -0.015);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, -0.060, -0.015);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.011, -0.060, -0.011);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, -0.105, -0.009);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.006, -0.105, -0.006);
	glNormal3d(-0.000, 0.000, -1.000);
	glVertex3d(-0.000, -0.120, -0.000);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.000, -0.120, -0.000);
	glEnd();

	glBegin(GL_QUAD_STRIP);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.000, 0.081, -0.000);
	glNormal3d(1.000, 0.000, -0.000);
	glVertex3d(0.000, 0.081, -0.000);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.005, 0.078, -0.005);
	glNormal3d(1.000, 0.000, -0.000);
	glVertex3d(0.007, 0.078, -0.000);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.008, 0.072, -0.008);
	glNormal3d(1.000, 0.000, -0.000);
	glVertex3d(0.012, 0.072, -0.000);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.011, 0.060, -0.011);
	glNormal3d(1.000, 0.000, -0.000);
	glVertex3d(0.015, 0.060, -0.000);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.014, 0.030, -0.014);
	glNormal3d(1.000, 0.000, -0.000);
	glVertex3d(0.019, 0.030, -0.000);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.015, 0.000, -0.015);
	glNormal3d(1.000, 0.000, -0.000);
	glVertex3d(0.021, 0.000, -0.000);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.015, -0.030, -0.015);
	glNormal3d(1.000, 0.000, -0.000);
	glVertex3d(0.021, -0.030, -0.000);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.011, -0.060, -0.011);
	glNormal3d(1.000, 0.000, -0.000);
	glVertex3d(0.015, -0.060, -0.000);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.006, -0.105, -0.006);
	glNormal3d(1.000, 0.000, -0.000);
	glVertex3d(0.009, -0.105, -0.000);
	glNormal3d(0.707, 0.000, -0.707);
	glVertex3d(0.000, -0.120, -0.000);
	glNormal3d(1.000, 0.000, -0.000);
	glVertex3d(0.000, -0.120, -0.000);
	glEnd();

	glEndList();
}
