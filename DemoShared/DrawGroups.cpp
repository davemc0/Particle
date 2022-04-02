// DrawGroups.cpp
//
// Copyright 1998 by David K. McAllister
//
// This file draws particle groups in OpenGL.

// OpenGL
#include "GL/glew.h"

// This needs to come after GLEW
#include "GL/freeglut.h"

#include "Particle/pAPI.h"

using namespace PAPI;

// Emit OpenGL calls to draw the particles as GL_LINES.
// The color is set per primitive or is constant.
// The other vertex of the line is the velocity vector.
void DrawGroupAsLines(ParticleContext_t &P, bool const_color)
{
    int cnt = (int)P.GetGroupCount();
    if(cnt < 1) return;

    float *ptr;
    size_t flstride, pos3Ofs, posB3Ofs, size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs, up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs;
;

    cnt = (int)P.GetParticlePointer(ptr, flstride, pos3Ofs, posB3Ofs,
        size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs,
        up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs);
    if(cnt < 1) return;

    glBegin(GL_LINES);

    if(!const_color) {
        for(int i = 0; i < cnt; i++) {
            // Warning: this depends on alpha following color in the Particle struct.
            glColor4fv((GLfloat *)ptr + flstride*i + color3Ofs);
            glVertex3fv((GLfloat *)ptr + flstride*i + pos3Ofs);

            // Make a tail with the velocity vector's direction and length.
            pVec tail = (*(pVec *)(ptr + flstride*i + pos3Ofs)) - (*(pVec *)(ptr + flstride*i + vel3Ofs));
            glVertex3fv((GLfloat *)&tail);
        }
    } else {
        for(int i = 0; i < cnt; i++) {
            glVertex3fv((GLfloat *)ptr + flstride*i + pos3Ofs);

            // Make a tail with the velocity vector's direction and length.
            pVec tail = (*(pVec *)(ptr + flstride*i + pos3Ofs)) - (*(pVec *)(ptr + flstride*i + vel3Ofs));
            glVertex3fv((GLfloat *)&tail);
        }
    }
    glEnd();
}

// Draw each particle by translating, scaling, and rotating the display list
// to the position and orientation of the particle. Also sets the glColor
// before calling the display list for each particle.
void DrawGroupAsDisplayLists(ParticleContext_t &P, int dlist, bool const_color, bool const_rotation)
{
    int cnt = (int)P.GetGroupCount();
    if(cnt < 1) return;

    float *ptr;
    size_t flstride, pos3Ofs, posB3Ofs, size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs, up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs;

    cnt = (int)P.GetParticlePointer(ptr, flstride, pos3Ofs, posB3Ofs,
        size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs,
        up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs);

    for(int i = 0; i < cnt; i++) {
        glPushMatrix();
        pVec &m_pos = *(pVec *)ptr;
        glTranslatef(m_pos.x(), m_pos.y(), m_pos.z());

        pVec &m_size = *(pVec *)(ptr + size3Ofs);
        glScalef(m_size.x(), m_size.y(), m_size.z());

        if(!const_rotation) {
            // Expensive!
            // velB stores the velocity from last frame.
            // vel ^ velB points to the side.
            pVec &m_vel = *(pVec *)(ptr + vel3Ofs);
            pVec &m_velB = *(pVec *)(ptr + velB3Ofs);

            pVec vN(m_vel);
            vN.normalize();
            pVec voN(m_velB);
            voN.normalize();

            pVec biN;
            if(voN.x() == vN.x() && voN.y() == vN.y() && voN.z() == vN.z())
                biN = pVec(0, 1, 0);
            else
                biN = Cross(vN, voN);
            biN.normalize();

            pVec N = Cross(vN, biN);

            float M[16];
            // clang-format off
            M[0] = vN.x();  M[4] = biN.x();  M[ 8] = N.x();  M[12] = 0;
            M[1] = vN.y();  M[5] = biN.y();  M[ 9] = N.y();  M[13] = 0;
            M[2] = vN.z();  M[6] = biN.z();  M[10] = N.z();  M[14] = 0;
            M[3] = 0;       M[7] = 0;        M[11] = 0;      M[15] = 1;
            // clang-format on
            glMultMatrixf(M);
        }

        // Warning: this depends on alpha following color in the Particle struct.
        if(!const_color)
            glColor4fv((GLfloat *)(ptr + color3Ofs));

        glCallList(dlist);

        glPopMatrix();
        ptr += flstride;
    }
}

// Draw each particle as a screen-aligned triangle with texture.
// Doesn't make the texture current. Just emits texcoords, if specified.
// If size_scale is 1 and const_size is true then the textured square
// will be 2x2 in world space (making the triangle sides be 4x4).

// ViewV and UpV must be normalized and unequal.
// The triangle is twice the screen area as the quad and thus takes twice
// the rasterization and shading time.
// However, the quad has four vertices whereas the tri has 3, so the quad
// takes more geometry processing time.
void DrawGroupAsTriSprites(ParticleContext_t &P, const pVec &view, const pVec &up,
                           float size_scale = 1.0f, bool draw_tex=false,
                           bool const_size=false, bool const_color=false)
{
    int cnt = (int)P.GetGroupCount();

    if(cnt < 1)
        return;

    pVec *ppos = new pVec[cnt];
    float *color = const_color ? NULL : new float[cnt * 4];
    pVec *size = const_size ? NULL : new pVec[cnt];

    P.GetParticles(0, cnt, (float *)ppos, color, NULL, (float *)size);

    // Compute the vectors from the particle to the corners of its tri.
    // 2
    // |\ The particle is at the center of the x.
    // |-\ V0, V1, and V2 go from there to the vertices.
    // |x|\ The texcoords are (0,0), (2,0), and (0,2) respectively.
    // 0-+-1 We clamp the texture so the rest is transparent.

    pVec right = Cross(view, up);
    right.normalize();
    pVec nup = Cross(right, view);
    right *= size_scale;
    nup *= size_scale;

    pVec V0 = -(right + nup);
    pVec V1 = V0 + right * 4;
    pVec V2 = V0 + nup * 4;

    glBegin(GL_TRIANGLES);

    for(int i = 0; i < cnt; i++) {
        pVec &p = ppos[i];

        if(!const_color)
            glColor4fv((GLfloat *)&color[i*4]);

        pVec sV0 = V0;
        pVec sV1 = V1;
        pVec sV2 = V2;

        if(!const_size)
        {
            sV0 *= size[i].x();
            sV1 *= size[i].x();
            sV2 *= size[i].x();
        }

        if(draw_tex) glTexCoord2f(0,0);
        pVec ver = p + sV0;
        glVertex3fv((GLfloat *)&ver);

        if(draw_tex) glTexCoord2f(2,0);
        ver = p + sV1;
        glVertex3fv((GLfloat *)&ver);

        if(draw_tex) glTexCoord2f(0,2);
        ver = p + sV2;
        glVertex3fv((GLfloat *)&ver);
    }

    glEnd();

    delete [] ppos;
    if(color) delete [] color;
    if(size) delete [] size;
}

// Draw each particle as a screen-aligned quad with texture.
// Doesn't make the texture current. Just emits texcoords, if specified.
// If size_scale is 1 and const_size is true then the textured square
// will be 2x2 in world space.

// ViewV and UpV must be normalized and unequal.
// The triangle is twice the screen area as the quad and thus takes twice
// the rasterization and shading time.
// However, the quad has four vertices whereas the tri has 3, so the quad
// takes more geometry processing time.

// Draw each particle as a screen-aligned quad with texture.
void DrawGroupAsQuadSprites(ParticleContext_t &P, const pVec &view, const pVec &up,
                            float size_scale = 1.0f, bool draw_tex=false,
                            bool const_size=false, bool const_color=false)
{
    int cnt = (int)P.GetGroupCount();

    if(cnt < 1)
        return;

    pVec *ppos = new pVec[cnt];
    float *color = const_color ? NULL : new float[cnt * 4];
    pVec *size = const_size ? NULL : new pVec[cnt];

    P.GetParticles(0, cnt, (float *)ppos, color, NULL, (float *)size);

    // Compute the vectors from the particle to the corners of its quad.
    // The particle is at the center of the x.
    // 3-2 V0, V1, V2 and V3 go from there to the vertices.
    // |x| The texcoords are (0,0), (1,0), (1,1), and (0,1) respectively.
    // 0-1 We clamp the texture so the rest is transparent.

    pVec right = Cross(view, up);
    right.normalize();
    pVec nup = Cross(right, view);
    right *= size_scale;
    nup *= size_scale;

    pVec V0 = -(right + nup);
    pVec V1 = right - nup;
    pVec V2 = right + nup;
    pVec V3 = nup - right;

    //cerr << "x " << view.x() << " " << view.y() << " " << view.z() << std::endl;
    //cerr << "x " << nup.x() << " " << nup.y() << " " << nup.z() << std::endl;
    //cerr << "x " << right.x() << " " << right.y() << " " << right.z() << std::endl;
    //cerr << "x " << V0.x() << " " << V0.y() << " " << V0.z() << std::endl;

    glBegin(GL_QUADS);

    for(int i = 0; i < cnt; i++)
    {
        pVec &p = ppos[i];
        //cerr << p.x() << " " << p.y() << " " << p.z() << std::endl;
        // std::cerr << color[i*4+3] << std::endl;

        if(!const_color)
            glColor4fv((GLfloat *)&color[i*4]);

        pVec sV0 = V0;
        pVec sV1 = V1;
        pVec sV2 = V2;
        pVec sV3 = V3;

        if(!const_size)
        {
            sV0 *= size[i].x();
            sV1 *= size[i].x();
            sV2 *= size[i].x();
            sV3 *= size[i].x();
        }

        if(draw_tex) glTexCoord2f(0,0);
        pVec ver = p + sV0;
        glVertex3fv((GLfloat *)&ver);

        if(draw_tex) glTexCoord2f(1,0);
        ver = p + sV1;
        glVertex3fv((GLfloat *)&ver);

        if(draw_tex) glTexCoord2f(1,1);
        ver = p + sV2;
        glVertex3fv((GLfloat *)&ver);

        if(draw_tex) glTexCoord2f(0,1);
        ver = p + sV3;
        glVertex3fv((GLfloat *)&ver);
    }

    glEnd();

    delete [] ppos;
    if(color) delete [] color;
    if(size) delete [] size;
}

// Draw as points using vertex arrays
// To draw as textured point sprites just call
// glEnable(GL_POINT_SPRITE_ARB) before calling this function.
void DrawGroupAsPoints(ParticleContext_t &P, const bool const_color)
{
    int cnt = (int)P.GetGroupCount();
    if(cnt < 1) return;

    float *ptr;
    size_t flstride, pos3Ofs, posB3Ofs, size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs, up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs;

    cnt = (int)P.GetParticlePointer(ptr, flstride, pos3Ofs, posB3Ofs,
        size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs,
        up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs);
    if(cnt < 1) return;

    if(!const_color) {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_FLOAT, int(flstride) * sizeof(float), ptr + color3Ofs);
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, int(flstride) * sizeof(float), ptr + pos3Ofs);

    glDrawArrays(GL_POINTS, 0, cnt);
    glDisableClientState(GL_VERTEX_ARRAY);
    if(!const_color)
        glDisableClientState(GL_COLOR_ARRAY);
}
