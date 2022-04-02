#if 0

#include "Model/Model.h"

// OpenGL
#include "GL/glew.h"

// This needs to come after GLEW
#include "GL/freeglut.h"

struct GLObject : public RenderObject
{
    inline GLObject(const RenderObject& O) : RenderObject(O) {}
  void RenderInOpenGL();
};

void GLObject::RenderInOpenGL()
{
  // Set up the per-object state.
  float duh[4] = {0, 0, 0, 1};

  GL_ASSERT();
  if(ShininessValid) glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
  if(SColorValid)
    {duh[0]=scolor.x; duh[1]=scolor.y; duh[2]=scolor.z;
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, duh); }
  if(EColorValid)
    {duh[0]=ecolor.x; duh[1]=ecolor.y; duh[2]=ecolor.z;
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, duh); }
  if(AColorValid)
    {duh[0]=acolor.x; duh[1]=acolor.y; duh[2]=acolor.z;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, duh); }

  GL_ASSERT();

  if(TexPtr->TexID >= 0)
    {
      int TexID = Model::TexDB.TexList[TexPtr->TexID]->TexID;

      ASSERT_R(glIsTexture(TexID));

      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, TexID);
      std::std::cerr << "Texture object ID = " << TexID << std::'\n';

      // Assume texcoords are actually 2D and ignore the third component.
      if(texcoords.size() == 1)
	glTexCoord2dv((GLdouble *) &texcoords[0]);
      else if(texcoords.size() > 0)
	{
	  ASSERT_R(texcoords.size() == verts.size());
	  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	  glTexCoordPointer(2, GL_DOUBLE, 3*sizeof(double), &texcoords[0]);
	}
    }
  else
    glDisable(GL_TEXTURE_2D);

  GL_ASSERT();

  if(normals.size() == 1)
    glNormal3dv((GLdouble *) &normals[0]);
  else if(normals.size() > 0)
    {
      ASSERT_R(normals.size() == verts.size());
      glEnableClientState(GL_NORMAL_ARRAY);
      glNormalPointer(GL_DOUBLE, 0, &normals[0]);
    }
		
  GL_ASSERT();

  if(dcolors.size() == 1)
    {
      // std::cerr << "One color\n";
      glColor3dv((GLdouble *) &dcolors[0]);
      if(alpha.size() == 1)
	glColor4f(dcolors[0].x, dcolors[0].y, dcolors[0].z, alpha[0]);
    }
  else if(dcolors.size() > 0)
    {
      ASSERT_R(dcolors.size() == verts.size());
      glEnableClientState(GL_COLOR_ARRAY);
      if(alpha.size() == verts.size())
	{
	  // Alpha is defined separately. Must fix it.
	  float *Colors = new float[dcolors.size()*4];
	  for(int i=0,j=0; i<dcolors.size(); i++)
	    {
	      Colors[j++] = dcolors[i].x;
	      Colors[j++] = dcolors[i].y;
	      Colors[j++] = dcolors[i].z;
	      Colors[j++] = alpha[i];
	    }
	  glColorPointer(4, GL_FLOAT, 0, Colors);
	  delete [] Colors;
	}
      else
	glColorPointer(3, GL_DOUBLE, 0, &dcolors[0]);
    }
  else
    {
      // std::cerr << "No colors.\n";
    }

  // First define the vertex arrays.
  glVertexPointer(3, GL_DOUBLE, 0, &verts[0]);

  // Use the vertices in order.
  glDrawArrays((GLenum)PrimType, 0, verts.size());
		
  GL_ASSERT();
}

// Render the given model in OpenGL.
void RenderModel(Model &M)
{
  glEnableClientState(GL_VERTEX_ARRAY);

  for(int i=0; i<M.Objs.size(); i++)
    {
      GLObject(M.Objs[i]).RenderInOpenGL();
    }
}

//The following are unused.

void LoadTexture(int TexID)
{
  if(TexID < 0)
    // This object has no texture.
    return;

  // Must get a texid, load the texture, and store the ID in the texture DB.

  unsigned int tid;
  glGenTextures(1, &tid);
  glBindTexture(GL_TEXTURE_2D, tid);

  // Store the tid in the TexDB.
  Model::TexDB.TexList[TexID].TexID = tid;

  Image *TexIm = Model::TexDB.TexList[TexID].Im;
  std::cerr << "Texture size is " << TexIm->wid<<"x"<<TexIm->hgt<<" channels="
       << TexIm->chan<<" bytes=" << TexIm->dsize << " texid="<<tid << '\n';

  // Convert it to 3 or 4 channels if it was 1 or 2.
  if(TexIm->chan <= 2)
    TexIm->Set(-1, -1, TexIm->chan + 2);

  // XXX Could this ever happen twice with the same image?
  TexIm->VFlip();

#if 1
    gluBuild2DMipmaps(GL_TEXTURE_2D, TexIm->chan, TexIm->wid, TexIm->hgt,
		      (TexIm->chan == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, TexIm->Pix);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, TexIm->chan, TexIm->wid, TexIm->hgt, 0,
		 (TexIm->chan == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, TexIm->Pix);
#endif

  ASSERT_R(glIsTexture(tid));
}

// Load the model's textures into OpenGL.
void LoadTextures(Model &M)
{
  // M.TexDB.Dump();
  for(int i=0; i<M.Objs.size(); i++)
      LoadTexture(M.Objs[i].TexID);

  // M.TexDB.Dump();
}

#endif