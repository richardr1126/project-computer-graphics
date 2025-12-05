/*
 *  Bullseye object - implementation file
 *  Contains drawing functions for bullseye targets
 */

#include "bullseye.h"
#include "../utils.h"

/*
 *  Aim a bullseye so its face points toward a target point in XZ
 *  Used to keep targets oriented toward the player's starting area
 *  @param b pointer to Bullseye structure
 *  @param tx target X coordinate
 *  @param tz target Z coordinate
 */
static void orientTowardXZ(Bullseye *b, double tx, double tz) {
  if (!b) return;
  double vx = tx - b->x;
  double vz = tz - b->z;
  double len = Vec3Length(vx, 0.0, vz);
  if (len < 1e-6) {
    b->dx = 1.0;
    b->dy = 0.0;
    b->dz = 0.0;
  } else {
    vx /= len;
    vz /= len;
    b->dx = vz;
    b->dy = 0.0;
    b->dz = -vx;
  }
  b->ux = 0.0;
  b->uy = 1.0;
  b->uz = 0.0;
}

/*
 *  Apply translation and orientation for a bullseye given Bullseye struct
 *  @param b pointer to Bullseye structure
 */
static void applyBullseyeTransform(const Bullseye *b) {
  if (!b)
    return;
  // Translate to (x,y,z), orient by (dx,dy,dz) and (ux,uy,uz)
  glTranslatef((float)b->x, (float)b->y, (float)b->z);

  double forwardX = b->dx;
  double forwardY = b->dy;
  double forwardZ = b->dz;
  double forwardLen = Vec3Length(forwardX, forwardY, forwardZ);
  if (forwardLen < 1e-6) {
    forwardX = 1.0;
    forwardY = forwardZ = 0.0;
  } else {
    forwardX /= forwardLen;
    forwardY /= forwardLen;
    forwardZ /= forwardLen;
  }

  double upX = b->ux;
  double upY = b->uy;
  double upZ = b->uz;
  double upLen = Vec3Length(upX, upY, upZ);
  if (upLen < 1e-6) {
    upX = 0.0;
    upY = 1.0;
    upZ = 0.0;
  } else {
    upX /= upLen;
    upY /= upLen;
    upZ /= upLen;
  }

  double rightX, rightY, rightZ;
  Vec3Cross(forwardX, forwardY, forwardZ, upX, upY, upZ, &rightX, &rightY, &rightZ);
  double rightLen = Vec3Length(rightX, rightY, rightZ);
  if (rightLen > 1e-6) {
    rightX /= rightLen;
    rightY /= rightLen;
    rightZ /= rightLen;
  } else {
    Vec3Cross(forwardX, forwardY, forwardZ, 0.0, 1.0, 0.0, &rightX, &rightY, &rightZ);
    Vec3Normalize(&rightX, &rightY, &rightZ);
  }

  double mat[16] = {
      forwardX, forwardY, forwardZ, 0.0,  // local +X
      upX,      upY,      upZ,      0.0,  // local +Y
      rightX,   rightY,   rightZ,   0.0,  // local +Z
      0.0,      0.0,      0.0,      1.0,
  };

  glMultMatrixd(mat);
}

/*
 *  Draw a bullseye from a Bullseye struct
 *  @param b pointer to Bullseye structure
 *  @param texture texture ID
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
 *  Draw normals for a bullseye
 *  (generated by AI)
 *  @param b pointer to Bullseye structure
 */
static void drawBullseyeNormals(const Bullseye *b) {
  if (!b) return;

  // Save transformation
  glPushMatrix();
  // Translate to (x,y,z), orient by (dx,dy,dz) and (ux,uy,uz)
  applyBullseyeTransform(b);

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
 *  @param b pointer to Bullseye structure
 *  @param texture texture ID
 *  @param showNormals 1 to draw normals, 0 otherwise
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
 *  @param zh z angle (degrees)
 *  @param showNormals 1 to draw normals, 0 otherwise
 *  @param texture texture ID
 */
void drawBullseyeScene(double zh, int showNormals, unsigned int texture) {
  const double aimX = 0.0;
  const double aimZ = 30.0;
  double off = 3.0 * Sin(zh);

  Bullseye b1 = {
    .x = 0.0, .y = 0.0, .z = -1.5 + 0.4 * off,
    .radius = 2.0,
    .rings = 6,
    .r = 1.0, .g = 0.0, .b = 0.0
  };
  orientTowardXZ(&b1, aimX, aimZ);
  drawBullseyeWithNormals(&b1, texture, showNormals);

  Bullseye b2 = {
    .x = -8.0 + 0.6 * off, .y = 5.0, .z = -4.0 - 0.4 * off,
    .radius = 1.25,
    .rings = 5,
    .r = 0.0, .g = 0.0, .b = 1.0
  };
  orientTowardXZ(&b2, aimX, aimZ);
  drawBullseyeWithNormals(&b2, texture, showNormals);

  Bullseye b3 = {
    .x = 8.0 - 0.6 * off, .y = 5.0, .z = -4.0 + 0.4 * off,
    .radius = 1.25,
    .rings = 4,
    .r = 0.0, .g = 1.0, .b = 0.0
  };
  orientTowardXZ(&b3, aimX, aimZ);
  drawBullseyeWithNormals(&b3, texture, showNormals);

  Bullseye b4 = {
    .x = -7.0 + 0.5 * off, .y = 1.75, .z = 5.5 + 0.7 * off,
    .radius = 1.25,
    .rings = 4,
    .r = 1.0, .g = 0.0, .b = 1.0
  };
  orientTowardXZ(&b4, aimX, aimZ);
  drawBullseyeWithNormals(&b4, texture, showNormals);

  Bullseye b5 = {
    .x = 7.5 - 0.5 * off, .y = 2.25, .z = 6.0 + 0.6 * off,
    .radius = 1.25,
    .rings = 5,
    .r = 0.0, .g = 1.0, .b = 1.0
  };
  orientTowardXZ(&b5, aimX, aimZ);
  drawBullseyeWithNormals(&b5, texture, showNormals);
}
