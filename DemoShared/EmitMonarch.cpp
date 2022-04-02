// Output a VRML model of this hardcoded butterfly so I can get rid of the hardcoded one.

#include <Model/Model.h>
#include <Math/Matrix44.h>

#define GL_TRIANGLE_FAN 1
#define GL_QUAD_STRIP 2

int  mode = 0, cnt = 0;
Vector V0;
Vector V1;
Vector N0;
Vector N1;
Vector Np;

Object Ob;

Matrix44 Trans, Tlate;

void Emit(Vector v0, Vector v1, Vector v2, Vector n0, Vector n1, Vector n2)
{
  v0 = Trans * v0;
  v1 = Trans * v1;
  v2 = Trans * v2;

  v0 = Tlate * v0;
  v1 = Tlate * v1;
  v2 = Tlate * v2;

  n0 = Trans * n0;
  n1 = Trans * n1;
  n2 = Trans * n2;

  Ob.verts.push_back(v0);
  Ob.verts.push_back(v1);
  Ob.verts.push_back(v2);

  Ob.normals.push_back(n0);
  Ob.normals.push_back(n1);
  Ob.normals.push_back(n2);
}

void glRotatef(double ang, float x, float y, float z)
{
  Trans.LoadIdentity();
  Trans.Rotate(DtoR(ang), Vector(x,y,z));
  cerr << Trans << endl;
}

void glLoadIdentity()
{
  Trans.LoadIdentity();
}

void glBegin(int m)
{
  mode = m;

  cnt = 0;

}

void glEnd() {}

void glColor3f(float x, float y, float z)
{
}

void glNormal3f(float x, float y, float z)
{
  Np = Vector(x,y,z);
}

void glVertex3f(float x, float y, float z)
{
  Vector V(x,y,z);

  switch(mode)
    {
    case GL_TRIANGLE_FAN:
      {
	if(cnt == 0)
	  {V0 = V; N0 = Np;}
	else if(cnt == 1)
	  {V1 = V; N1 = Np;}
	else
	  {
	    Emit(V0, V1, V, N0, N1, Np);
	    V1 = V; N1 = Np;
	  }
      }
      break;
    case GL_QUAD_STRIP:
      {
	if(cnt == 0)
	  {V0 = V; N0 = Np;}
	else if(cnt == 1)
	  {V1 = V; N1 = Np;}
	else if(cnt % 2)
	  {
	    Emit(V1, V0, V, N1, N0, Np);
	    V0 = V1; N0 = N1;
	    V1 = V; N1 = Np;
	  }
	else
	  {
	    Emit(V0, V1, V, N0, N1, Np);
	    V0 = V1; N0 = N1;
	    V1 = V; N1 = Np;
	  }
      }
      break;
    }

  cnt++;
}

// Make a Monarch Butterfly by hand. I can't believe I ever did this.
// This one's for you, Tiffany!
void monarch()
{
  glRotatef(360/16., 0, 1, 0);

  // Left Wing
  glBegin(GL_TRIANGLE_FAN);

  glNormal3f(0.0, 0.0, 1.0);
  glVertex3f(-0.075, 0.0, 0.0);
	
  glNormal3f(0.707, 0.707, 0.000);
  glVertex3f(-0.015, 0.0, 0.0);
  glNormal3f(0.659, 0.707, 0.255);
  glVertex3f(-0.019, 0.030, 0.0);
  glNormal3f(0.523, 0.707, 0.476);
  glVertex3f(-0.015, 0.060, 0.0);
  glNormal3f(0.315, 0.707, 0.633);
  glVertex3f(-0.045, 0.069, 0.0);
  glNormal3f(0.065, 0.707, 0.704);
  glVertex3f(-0.075, 0.075, 0.0);
  glNormal3f(-0.194, 0.707, 0.680);
  glVertex3f(-0.105, 0.069, 0.0);
  glNormal3f(-0.426, 0.707, 0.564);
  glVertex3f(-0.135, 0.060, 0.0);
  glNormal3f(-0.601, 0.707, 0.372);
  glVertex3f(-0.150, 0.030, 0.0);
  glNormal3f(-0.695, 0.707, 0.130);
  glVertex3f(-0.141, 0.0, 0.0);
  glNormal3f(-0.695, 0.707, -0.130);
  glVertex3f(-0.126, -0.030, 0.0);
  glNormal3f(-0.601, 0.707, -0.372);
  glVertex3f(-0.108, -0.060, 0.0);
  glNormal3f(-0.426, 0.707, -0.564);
  glVertex3f(-0.075, -0.084, 0.0);
  glNormal3f(-0.194, 0.707, -0.680);
  glVertex3f(-0.045, -0.129, 0.0);
  glNormal3f(0.065, 0.707, -0.704);
  glVertex3f(-0.030, -0.120, 0.0);
  glNormal3f(0.315, 0.707, -0.633);
  glVertex3f(-0.009, -0.105, 0.0);
  glNormal3f(0.523, 0.707, -0.476);
  glVertex3f(-0.015, -0.060, 0.0);
  glNormal3f(0.659, 0.707, -0.255);
  glVertex3f(-0.015, -0.030, 0.0);
  glNormal3f(0.707, 0.707, -0.000);
  glVertex3f(-0.015, -0.0, 0.0);
	
  glEnd();

  glLoadIdentity();
	
  // Right Wing
  glRotatef(-360/16.0, 0, 1, 0);

  glBegin(GL_TRIANGLE_FAN);
	
  glNormal3f(0.0, 0.0, 1.0);
  glVertex3f(0.075, 0.0, 0.0);
	
  glNormal3f(0.707, 0.707, 0.000);
  glVertex3f(0.015, -0.0, 0.0);
  glNormal3f(0.659, 0.707, 0.255);
  glVertex3f(0.015, -0.030, 0.0);
  glNormal3f(0.523, 0.707, 0.476);
  glVertex3f(0.015, -0.060, 0.0);
  glNormal3f(0.315, 0.707, 0.633);
  glVertex3f(0.009, -0.105, 0.0);
  glNormal3f(0.065, 0.707, 0.704);
  glVertex3f(0.030, -0.120, 0.0);
  glNormal3f(-0.194, 0.707, 0.680);
  glVertex3f(0.045, -0.129, 0.0);
  glNormal3f(-0.426, 0.707, 0.564);
  glVertex3f(0.075, -0.084, 0.0);
  glNormal3f(-0.601, 0.707, 0.372);
  glVertex3f(0.108, -0.060, 0.0);
  glNormal3f(-0.695, 0.707, 0.130);
  glVertex3f(0.126, -0.030, 0.0);
  glNormal3f(-0.695, 0.707, -0.130);
  glVertex3f(0.141, 0.0, 0.0);
  glNormal3f(-0.601, 0.707, -0.372);
  glVertex3f(0.150, 0.030, 0.0);
  glNormal3f(-0.426, 0.707, -0.564);
  glVertex3f(0.135, 0.060, 0.0);
  glNormal3f(-0.194, 0.707, -0.680);
  glVertex3f(0.105, 0.069, 0.0);
  glNormal3f(0.065, 0.707, -0.704);
  glVertex3f(0.075, 0.075, 0.0);
  glNormal3f(0.315, 0.707, -0.633);
  glVertex3f(0.045, 0.069, 0.0);
  glNormal3f(0.523, 0.707, -0.476);
  glVertex3f(0.015, 0.060, 0.0);
  glNormal3f(0.659, 0.707, -0.255);
  glVertex3f(0.019, 0.030, 0.0);
  glNormal3f(0.707, 0.707, -0.000);
  glVertex3f(0.015, 0.0, 0.0);
	
  glEnd();
	
  glColor3f(.5, .5, 1);

  // Body contour
  glBegin(GL_QUAD_STRIP);
  glNormal3f(1.000, 0.000, 0.000);
  glVertex3f(0.000, 0.081, 0.000);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.000, 0.081, 0.000);
  glNormal3f(1.000, 0.000, 0.000);
  glVertex3f(0.007, 0.078, 0.000);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.005, 0.078, 0.005);
  glNormal3f(1.000, 0.000, 0.000);
  glVertex3f(0.012, 0.072, 0.000);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.008, 0.072, 0.008);
  glNormal3f(1.000, 0.000, 0.000);
  glVertex3f(0.015, 0.060, 0.000);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.011, 0.060, 0.011);
  glNormal3f(1.000, 0.000, 0.000);
  glVertex3f(0.019, 0.030, 0.000);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.014, 0.030, 0.014);
  glNormal3f(1.000, 0.000, 0.000);
  glVertex3f(0.021, 0.000, 0.000);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.015, 0.000, 0.015);
  glNormal3f(1.000, 0.000, 0.000);
  glVertex3f(0.021, -0.030, 0.000);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.015, -0.030, 0.015);
  glNormal3f(1.000, 0.000, 0.000);
  glVertex3f(0.015, -0.060, 0.000);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.011, -0.060, 0.011);
  glNormal3f(1.000, 0.000, 0.000);
  glVertex3f(0.009, -0.105, 0.000);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.006, -0.105, 0.006);
  glNormal3f(1.000, 0.000, 0.000);
  glVertex3f(0.000, -0.120, 0.000);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.000, -0.120, 0.000);
  glEnd();
	
  glBegin(GL_QUAD_STRIP);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.000, 0.081, 0.000);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, 0.081, 0.000);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.005, 0.078, 0.005);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, 0.078, 0.007);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.008, 0.072, 0.008);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, 0.072, 0.012);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.011, 0.060, 0.011);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, 0.060, 0.015);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.014, 0.030, 0.014);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, 0.030, 0.019);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.015, 0.000, 0.015);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, 0.000, 0.021);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.015, -0.030, 0.015);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, -0.030, 0.021);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.011, -0.060, 0.011);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, -0.060, 0.015);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.006, -0.105, 0.006);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, -0.105, 0.009);
  glNormal3f(0.707, 0.000, 0.707);
  glVertex3f(0.000, -0.120, 0.000);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, -0.120, 0.000);
  glEnd();
	
  glBegin(GL_QUAD_STRIP);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, 0.081, 0.000);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.000, 0.081, 0.000);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, 0.078, 0.007);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.005, 0.078, 0.005);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, 0.072, 0.012);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.008, 0.072, 0.008);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, 0.060, 0.015);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.011, 0.060, 0.011);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, 0.030, 0.019);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.014, 0.030, 0.014);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, 0.000, 0.021);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.015, 0.000, 0.015);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, -0.030, 0.021);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.015, -0.030, 0.015);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, -0.060, 0.015);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.011, -0.060, 0.011);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, -0.105, 0.009);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.006, -0.105, 0.006);
  glNormal3f(0.000, 0.000, 1.000);
  glVertex3f(0.000, -0.120, 0.000);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.000, -0.120, 0.000);
  glEnd();
	
  glBegin(GL_QUAD_STRIP);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.000, 0.081, 0.000);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.000, 0.081, 0.000);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.005, 0.078, 0.005);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.007, 0.078, 0.000);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.008, 0.072, 0.008);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.012, 0.072, 0.000);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.011, 0.060, 0.011);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.015, 0.060, 0.000);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.014, 0.030, 0.014);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.019, 0.030, 0.000);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.015, 0.000, 0.015);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.021, 0.000, 0.000);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.015, -0.030, 0.015);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.021, -0.030, 0.000);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.011, -0.060, 0.011);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.015, -0.060, 0.000);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.006, -0.105, 0.006);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.009, -0.105, 0.000);
  glNormal3f(-0.707, 0.000, 0.707);
  glVertex3f(-0.000, -0.120, 0.000);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.000, -0.120, 0.000);
  glEnd();
	
  glBegin(GL_QUAD_STRIP);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.000, 0.081, 0.000);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.000, 0.081, -0.000);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.007, 0.078, 0.000);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.005, 0.078, -0.005);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.012, 0.072, 0.000);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.008, 0.072, -0.008);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.015, 0.060, 0.000);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.011, 0.060, -0.011);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.019, 0.030, 0.000);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.014, 0.030, -0.014);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.021, 0.000, 0.000);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.015, 0.000, -0.015);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.021, -0.030, 0.000);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.015, -0.030, -0.015);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.015, -0.060, 0.000);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.011, -0.060, -0.011);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.009, -0.105, 0.000);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.006, -0.105, -0.006);
  glNormal3f(-1.000, 0.000, 0.000);
  glVertex3f(-0.000, -0.120, 0.000);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.000, -0.120, -0.000);
  glEnd();
	
  glBegin(GL_QUAD_STRIP);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.000, 0.081, -0.000);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, 0.081, -0.000);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.005, 0.078, -0.005);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, 0.078, -0.007);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.008, 0.072, -0.008);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, 0.072, -0.012);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.011, 0.060, -0.011);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, 0.060, -0.015);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.014, 0.030, -0.014);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, 0.030, -0.019);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.015, 0.000, -0.015);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, 0.000, -0.021);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.015, -0.030, -0.015);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, -0.030, -0.021);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.011, -0.060, -0.011);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, -0.060, -0.015);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.006, -0.105, -0.006);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, -0.105, -0.009);
  glNormal3f(-0.707, 0.000, -0.707);
  glVertex3f(-0.000, -0.120, -0.000);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, -0.120, -0.000);
  glEnd();
	
  glBegin(GL_QUAD_STRIP);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, 0.081, -0.000);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.000, 0.081, -0.000);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, 0.078, -0.007);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.005, 0.078, -0.005);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, 0.072, -0.012);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.008, 0.072, -0.008);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, 0.060, -0.015);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.011, 0.060, -0.011);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, 0.030, -0.019);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.014, 0.030, -0.014);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, 0.000, -0.021);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.015, 0.000, -0.015);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, -0.030, -0.021);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.015, -0.030, -0.015);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, -0.060, -0.015);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.011, -0.060, -0.011);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, -0.105, -0.009);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.006, -0.105, -0.006);
  glNormal3f(-0.000, 0.000, -1.000);
  glVertex3f(-0.000, -0.120, -0.000);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.000, -0.120, -0.000);
  glEnd();
	
  glBegin(GL_QUAD_STRIP);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.000, 0.081, -0.000);
  glNormal3f(1.000, 0.000, -0.000);
  glVertex3f(0.000, 0.081, -0.000);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.005, 0.078, -0.005);
  glNormal3f(1.000, 0.000, -0.000);
  glVertex3f(0.007, 0.078, -0.000);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.008, 0.072, -0.008);
  glNormal3f(1.000, 0.000, -0.000);
  glVertex3f(0.012, 0.072, -0.000);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.011, 0.060, -0.011);
  glNormal3f(1.000, 0.000, -0.000);
  glVertex3f(0.015, 0.060, -0.000);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.014, 0.030, -0.014);
  glNormal3f(1.000, 0.000, -0.000);
  glVertex3f(0.019, 0.030, -0.000);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.015, 0.000, -0.015);
  glNormal3f(1.000, 0.000, -0.000);
  glVertex3f(0.021, 0.000, -0.000);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.015, -0.030, -0.015);
  glNormal3f(1.000, 0.000, -0.000);
  glVertex3f(0.021, -0.030, -0.000);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.011, -0.060, -0.011);
  glNormal3f(1.000, 0.000, -0.000);
  glVertex3f(0.015, -0.060, -0.000);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.006, -0.105, -0.006);
  glNormal3f(1.000, 0.000, -0.000);
  glVertex3f(0.009, -0.105, -0.000);
  glNormal3f(0.707, 0.000, -0.707);
  glVertex3f(0.000, -0.120, -0.000);
  glNormal3f(1.000, 0.000, -0.000);
  glVertex3f(0.000, -0.120, -0.000);
  glEnd();
}

int main()
{
#define SC 4.7619
  Tlate.Scale(Vector(SC,SC,SC));
  Tlate.Translate(Vector(0,0.1289999935,0));

  monarch();
  // Ob.GenNormals(1.57);

  Model Mo(Ob);
  Mo.RebuildBBox();

  cerr << "Box: " << Mo.Box << endl;
  Mo.Save("monarch.obj");
}
