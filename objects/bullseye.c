/*
 *  Bullseye object - implementation file
 *  Contains drawing functions for bullseye targets
 */

#include "bullseye.h"
#include "../utils.h"

/*
 *  Apply translation and orientation for a bullseye given Bullseye struct
 */
static void applyBullseyeTransform(const Bullseye *b) {
  if (!b) return;
  // Translate to (x,y,z), orient by (dx,dy,dz) and (ux,uy,uz)
  glTranslatef((float)b->x, (float)b->y, (float)b->z);
  // Build orthonormal basis from direction (dx,dy,dz) and up (ux,uy,uz)
  double dx = b->dx, dy = b->dy, dz = b->dz;
  double ux = b->ux, uy = b->uy, uz = b->uz;
  double D0 = sqrt(dx * dx + dy * dy + dz * dz);
  if (!D0) D0 = 1;
  double X0 = dx / D0, Y0 = dy / D0, Z0 = dz / D0; // local X axis
  double D1 = sqrt(ux * ux + uy * uy + uz * uz);
  if (!D1) D1 = 1;
  double X1 = ux / D1, Y1 = uy / D1, Z1 = uz / D1; // local Y axis
  // Cross to get local Z axis
  double X2 = Y0 * Z1 - Y1 * Z0;
  double Y2 = Z0 * X1 - Z1 * X0;
  double Z2 = X0 * Y1 - X1 * Y0;
  //  Rotation matrix (column-major for OpenGL)
  double mat[16];
  mat[0] = X0;   mat[4] = X1;   mat[ 8] = X2;   mat[12] = 0;
  mat[1] = Y0;   mat[5] = Y1;   mat[ 9] = Y2;   mat[13] = 0;
  mat[2] = Z0;   mat[6] = Z1;   mat[10] = Z2;   mat[14] = 0;
  mat[3] =  0;   mat[7] =  0;   mat[11] =  0;   mat[15] = 1;

  glMultMatrixd(mat);
}

/*
 *  Draw a bullseye from a Bullseye struct
 */
void drawBullseye(const Bullseye *b, unsigned int texture) {
  if (!b) return;
  // Save transformation
  glPushMatrix();
  // Translate to (x,y,z), orient by (dx,dy,dz) and (ux,uy,uz)
  applyBullseyeTransform(b);

  // Enable texturing if texture provided
  if (texture) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  }

  // Draw concentric rings (bullseye) as a short 3D stack
  int nRings = (b->rings > 0) ? b->rings : 1; // Number of colored rings (validated)
  const double R = (b->radius > 0.0) ? b->radius : 1.0; // Outer radius in world units
  const double step = R / nRings;
  const int d = 10;      // Angular step in degrees
  const double hz = 0.1; // Half-thickness in world units (constant)

  // Draw outer-to-inner rings, alternating colors
  for (int i = 0; i < nRings; ++i) {
    double ro = R - i * step; // outer radius of ring
    double ri = ro - step;    // inner radius of ring

    // Alternate colors: specified color/white per ring (colors blend with
    // texture via GL_MODULATE)
    if (i % 2 == 0)
      glColor3f(b->r, b->g, b->b);
    else
      glColor3f(1.0, 1.0, 1.0);

    if (ri > 0.0) {
      // Top face (z=+hz) - normals +Z
      glBegin(GL_TRIANGLE_STRIP);
      glNormal3f(0, 0, +1);
      for (int ang = 0; ang <= 360; ang += d) {
        double c = Cos(ang);
        double s = Sin(ang);
        if (texture) glTexCoord2d(0.5 + 0.5 * ro * c / R, 0.5 + 0.5 * ro * s / R);
        glVertex3d(ro * c, ro * s, +hz);
        if (texture) glTexCoord2d(0.5 + 0.5 * ri * c / R, 0.5 + 0.5 * ri * s / R);
        glVertex3d(ri * c, ri * s, +hz);
      }
      glEnd();
      // Bottom face (z=-hz) - normals -Z
      glBegin(GL_TRIANGLE_STRIP);
      glNormal3f(0, 0, -1);
      for (int ang = 0; ang <= 360; ang += d) {
        double c = Cos(ang);
        double s = Sin(ang);
        if (texture) glTexCoord2d(0.5 + 0.5 * ro * c / R, 0.5 + 0.5 * ro * s / R);
        glVertex3d(ro * c, ro * s, -hz);
        if (texture) glTexCoord2d(0.5 + 0.5 * ri * c / R, 0.5 + 0.5 * ri * s / R);
        glVertex3d(ri * c, ri * s, -hz);
      }
      glEnd();
      // Outer side wall (cylindrical surface at radius ro) - radial outward
      // normals
      glBegin(GL_QUAD_STRIP);
      for (int ang = 0; ang <= 360; ang += d) {
        double c = Cos(ang);
        double s = Sin(ang);
        glNormal3d(c, s, 0);
        if (texture) glTexCoord2d(ang / 360.0, 1.0);
        glVertex3d(ro * c, ro * s, +hz);
        glNormal3d(c, s, 0);
        if (texture) glTexCoord2d(ang / 360.0, 0.0);
        glVertex3d(ro * c, ro * s, -hz);
      }
      glEnd();
      // Inner side wall (cylindrical surface at radius ri) - radial inward
      // normals
      glBegin(GL_QUAD_STRIP);
      for (int ang = 0; ang <= 360; ang += d) {
        double c = Cos(ang);
        double s = Sin(ang);
        glNormal3d(-c, -s, 0);
        if (texture) glTexCoord2d(ang / 360.0, 0.0);
        glVertex3d(ri * c, ri * s, -hz);
        glNormal3d(-c, -s, 0);
        if (texture) glTexCoord2d(ang / 360.0, 1.0);
        glVertex3d(ri * c, ri * s, +hz);
      }
      glEnd();
    } else {
      // Center disk (no inner hole)
      // Top
      glBegin(GL_TRIANGLE_FAN);
      glNormal3f(0, 0, +1);
      if (texture) glTexCoord2d(0.5, 0.5);
      glVertex3d(0.0, 0.0, +hz);
      for (int ang = 0; ang <= 360; ang += d) {
        double c = Cos(ang);
        double s = Sin(ang);
        if (texture) glTexCoord2d(0.5 + 0.5 * ro * c / R, 0.5 + 0.5 * ro * s / R);
        glVertex3d(ro * c, ro * s, +hz);
      }
      glEnd();
      // Bottom
      glBegin(GL_TRIANGLE_FAN);
      glNormal3f(0, 0, -1);
      if (texture) glTexCoord2d(0.5, 0.5);
      glVertex3d(0.0, 0.0, -hz);
      for (int ang = 0; ang <= 360; ang += d) {
        double c = Cos(ang);
        double s = Sin(ang);
        if (texture) glTexCoord2d(0.5 + 0.5 * ro * c / R, 0.5 + 0.5 * ro * s / R);
        glVertex3d(ro * c, ro * s, -hz);
      }
      glEnd();
      // Side cylinder at radius ro
      glBegin(GL_QUAD_STRIP);
      for (int ang = 0; ang <= 360; ang += d) {
        double c = Cos(ang);
        double s = Sin(ang);
        glNormal3d(c, s, 0);
        if (texture) glTexCoord2d(ang / 360.0, 1.0);
        glVertex3d(ro * c, ro * s, +hz);
        glNormal3d(c, s, 0);
        if (texture) glTexCoord2d(ang / 360.0, 0.0);
        glVertex3d(ro * c, ro * s, -hz);
      }
      glEnd();
    }
  }

  // Disable texturing after drawing
  if (texture) glDisable(GL_TEXTURE_2D);

  // Restore transformation
  glPopMatrix();
}

/*
 *  Draw normals for a bullseye (generated by AI)
 */
static void drawBullseyeNormals(const Bullseye *b) {
  if (!b) return;

  // Save transformation
  glPushMatrix();
  // Translate to (x,y,z), orient by (dx,dy,dz) and (ux,uy,uz)
  applyBullseyeTransform(b);

  // Below was generated with AI assistance
  // Draws normals for a bullseye (unlit lines)
  const double step = b->radius / b->rings;
  const double hz = 0.1;
  const int d = 30;     // sparser for visibility
  const double L = 0.3; // line length
  glColor3f(1, 1, 0);   // Apply yellow color for normals
  glBegin(GL_LINES);
  // Top and bottom face normals (sample at a few angles on outermost ring)
  for (int ang = 0; ang < 360; ang += d) {
    double c = Cos(ang), s = Sin(ang);
    // top
    glVertex3d(b->radius * c, b->radius * s, +hz);
    glVertex3d(b->radius * c, b->radius * s, +hz + L);
    // bottom
    glVertex3d(b->radius * c, b->radius * s, -hz);
    glVertex3d(b->radius * c, b->radius * s, -hz - L);
  }
  // Side walls (outer and inner of one ring)
  for (int ang = 0; ang < 360; ang += d) {
    double c = Cos(ang), s = Sin(ang);
    // outer wall normal
    glVertex3d(b->radius * c, b->radius * s, 0);
    glVertex3d(b->radius * c + L * c, b->radius * s + L * s, 0);
    // inner wall normal on innermost radius
    double ri = (b->rings > 0) ? (b->radius - step) : (b->radius * 0.5);
    if (ri > 0) {
      glVertex3d(ri * c, ri * s, 0);
      glVertex3d(ri * c - L * c, ri * s - L * s, 0);
    }
  }
  glEnd();
  glPopMatrix();
}

/*
 *  Helper to draw normals for a bullseye if requested
 */
static void drawBullseyeWithNormals(const Bullseye *b, unsigned int texture, int showNormals) {
  drawBullseye(b, texture);
  if (showNormals) {
    GLboolean wasLit = glIsEnabled(GL_LIGHTING);
    if (wasLit)
      glDisable(GL_LIGHTING);
    drawBullseyeNormals(b);
    if (wasLit)
      glEnable(GL_LIGHTING);
  }
}

/*
 *  Draw the scene with multiple bullseye targets
 */
void drawBullseyeScene(double zh, int showNormals, unsigned int texture) {
  double off = 3.0 * Sin(zh);

  Bullseye b1 = {
    .x = 0.0, .y = 0.0, .z = 0.0,
    .dx = 1.0, .dy = 0.0, .dz = 0.0,
    .ux = 0.0, .uy = 1.0, .uz = 0.0,
    .radius = 2.0,
    .rings = 6,
    .r = 1.0, .g = 0.0, .b = 0.0
  };
  drawBullseyeWithNormals(&b1, texture, showNormals);

  double cos45 = Cos(45), sin45 = Sin(45);
  double moveX1 = cos45 * off;
  double moveZ1 = -sin45 * off;
  Bullseye b2 = {
    .x = -3.0 + moveX1, .y = 5.0, .z = -1.0 + moveZ1,
    .dx = cos45, .dy = 0.0, .dz = sin45,
    .ux = 0.0, .uy = 1.0, .uz = 0.0,
    .radius = 1.25,
    .rings = 5,
    .r = 0.0, .g = 0.0, .b = 1.0
  };
  drawBullseyeWithNormals(&b2, texture, showNormals);

  Bullseye b3 = {
    .x = 3.0 + off, .y = 5.0, .z = 1.0,
    .dx = Cos(-30), .dy = 0.0, .dz = Sin(-30),
    .ux = 0.0, .uy = 1.0, .uz = 0.0,
    .radius = 1.25,
    .rings = 4,
    .r = 0.0, .g = 1.0, .b = 0.0
  };
  drawBullseyeWithNormals(&b3, texture, showNormals);

  double cos60 = Cos(60), sin60 = Sin(60);
  double moveX3 = cos60 * off;
  double moveZ3 = -sin60 * off;
  Bullseye b4 = {
    .x = -3.0 + moveX3, .y = 1.5, .z = -1.0 + moveZ3,
    .dx = cos60, .dy = 0.0, .dz = sin60,
    .ux = 0.0, .uy = 1.0, .uz = 0.0,
    .radius = 1.25,
    .rings = 4,
    .r = 1.0, .g = 0.0, .b = 1.0
  };
  drawBullseyeWithNormals(&b4, texture, showNormals);

  Bullseye b5 = {
    .x = 3.0, .y = 2.5, .z = 1.0 + off,
    .dx = 0.0, .dy = 0.0, .dz = 1.0,
    .ux = 0.0, .uy = 1.0, .uz = 0.0,
    .radius = 1.25,
    .rings = 5,
    .r = 0.0, .g = 1.0, .b = 1.0
  };
  drawBullseyeWithNormals(&b5, texture, showNormals);
}
