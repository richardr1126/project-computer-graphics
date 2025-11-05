/*
 *  Lighting objects - implementation file
 *  Contains drawing functions for light representation
 */

#include "../utils.h"
#include "lighting.h"

/*
 *  Vertex on a unit sphere given angles (degrees)
 */
static void SphereVertex(double th, double ph)
{
    double x = Sin(th) * Cos(ph);
    double y = Cos(th) * Cos(ph);
    double z = Sin(ph);
    glNormal3d(x, y, z);
    // Texture coordinates: spherical mapping
    glTexCoord2d(th / 360.0, (ph + 90.0) / 180.0);
    glVertex3d(x, y, z);
}

/*
 *  Draw a lit sphere using latitude-longitude quads
 *  at (x,y,z) with radius r and angular increment inc degrees
 */
static void drawBall(double x, double y, double z, double r, int inc)
{
    if (inc < 1) inc = 1;
    // Save transform and move/scale
    glPushMatrix();
    glTranslated(x, y, z);
    glScaled(r, r, r);
    // Bands of latitude
    for (int ph = -90; ph < 90; ph += inc)
    {
        glBegin(GL_QUAD_STRIP);
        for (int th = 0; th <= 360; th += 2 * inc)
        {
            SphereVertex(th, ph);
            SphereVertex(th, ph + inc);
        }
        glEnd();
    }
    glPopMatrix();
}

/*
 *  Draw a small sphere to represent the light (unlit so it appears emissive)
 */
void drawLightBall(double x, double y, double z, double r)
{
    glPushMatrix();
    // Draw unlit so it appears emissive and not affected by scene lighting
    GLboolean wasLit = glIsEnabled(GL_LIGHTING);
    glDisable(GL_LIGHTING);
    
    // Draw as bright white/yellow color to represent light
    glColor3f(1, 1, 0.8);
    drawBall(x, y, z, r, 3);
    
    if (wasLit) glEnable(GL_LIGHTING);
    glPopMatrix();
}
