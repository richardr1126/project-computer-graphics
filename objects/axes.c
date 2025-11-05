/*
 *  Axes object - implementation file
 *  Contains drawing functions for coordinate axes
 */

#include "../utils.h"
#include "axes.h"

/*
 *  Draw axes
 *  Length is the axis length in world coordinates
 */
void drawAxes(double len)
{
    //  Save transformation
    glPushMatrix();

    //  Draw axes
    glBegin(GL_LINES);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(len, 0.0, 0.0);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, len, 0.0);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, 0.0, len);
    glEnd();

    //  Label axes
    glRasterPos3d(len, 0.0, 0.0);
    Print("X");
    glRasterPos3d(0.0, len, 0.0);
    Print("Y");
    glRasterPos3d(0.0, 0.0, len);
    Print("Z");

    //  Undo transformations
    glPopMatrix();
}
