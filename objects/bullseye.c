/*
 *  Bullseye object - implementation file
 *  Contains drawing functions for bullseye targets
 */

#include "bullseye.h"
#include "arrow.h"
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
 *  Get a bullseye definition by index
 *  @param index index of the bullseye (0 to N-1)
 *  @param zh animation angle
 *  @param b pointer to Bullseye structure to fill
 *  @return 1 if index is valid, 0 otherwise
 */
int getBullseye(int index, double zh, Bullseye *b) {
  const double aimX = 0.0;
  const double aimZ = 30.0;
  double off = 3.0 * Sin(zh);

  if (index == 0) {
    b->x = 0.0; b->y = 0.0; b->z = -1.5 + 0.4 * off;
    b->radius = 2.0; b->rings = 6;
    b->r = 1.0; b->g = 0.0; b->b = 0.0;
  } else if (index == 1) {
    b->x = -8.0 + 0.6 * off; b->y = 5.0; b->z = -4.0 - 0.4 * off;
    b->radius = 1.25; b->rings = 5;
    b->r = 0.0; b->g = 0.0; b->b = 1.0;
  } else if (index == 2) {
    b->x = 8.0 - 0.6 * off; b->y = 5.0; b->z = -4.0 + 0.4 * off;
    b->radius = 1.25; b->rings = 4;
    b->r = 0.0; b->g = 1.0; b->b = 0.0;
  } else if (index == 3) {
    b->x = -7.0 + 0.5 * off; b->y = 1.75; b->z = 5.5 + 0.7 * off;
    b->radius = 1.25; b->rings = 4;
    b->r = 1.0; b->g = 0.0; b->b = 1.0;
  } else if (index == 4) {
    b->x = 7.5 - 0.5 * off; b->y = 2.25; b->z = 6.0 + 0.6 * off;
    b->radius = 1.25; b->rings = 5;
    b->r = 0.0; b->g = 1.0; b->b = 1.0;
  } else {
    return 0;
  }
  
  orientTowardXZ(b, aimX, aimZ);
  return 1;
}

/*
 *  Draw the scene with multiple bullseye targets
 *  @param zh z angle (degrees)
 *  @param showNormals 1 to draw normals, 0 otherwise
 *  @param texture texture ID
 */
void drawBullseyeScene(double zh, int showNormals, unsigned int texture) {
  Bullseye b;
  int i = 0;
  while (getBullseye(i, zh, &b)) {
    drawBullseyeWithNormals(&b, texture, showNormals);
    i++;
  }
}


/*
 *  Check collision between arrow and bullseyes
 *  @param arrowPtr pointer to Arrow structure
 *  @param zh animation angle
 *  @return score (0 if no hit)
 */
int checkBullseyeCollision(void *arrowPtr, double zh) {
  Arrow *arrow = (Arrow *)arrowPtr;
  if (!arrow || !arrow->active || arrow->stuck) return 0;

  // Ray from prev to current (TIP of the arrow)
  // Arrow length = shaft (3.0) + tip (0.5) = 3.5
  const double arrowLen = 3.5;
  
  // Tip positions
  double tip1x = arrow->prevX + arrow->dx * arrowLen;
  double tip1y = arrow->prevY + arrow->dy * arrowLen;
  double tip1z = arrow->prevZ + arrow->dz * arrowLen;
  
  double tip2x = arrow->x + arrow->dx * arrowLen;
  double tip2y = arrow->y + arrow->dy * arrowLen;
  double tip2z = arrow->z + arrow->dz * arrowLen;

  double dirx = tip2x - tip1x, diry = tip2y - tip1y, dirz = tip2z - tip1z;
  double len = sqrt(dirx*dirx + diry*diry + dirz*dirz);
  if (len < 1e-6) return 0; // Not moving

  // Normalize direction
  double ndx = dirx / len, ndy = diry / len, ndz = dirz / len;

  Bullseye b;
  int i = 0;
  while (getBullseye(i, zh, &b)) {
    // Calculate correct normal vector for the bullseye plane
    double fx = b.dx, fy = b.dy, fz = b.dz;
    double ux = b.ux, uy = b.uy, uz = b.uz;
    double nx, ny, nz; // Normal
    Vec3Cross(fx, fy, fz, ux, uy, uz, &nx, &ny, &nz);
    Vec3Normalize(&nx, &ny, &nz);

    // Plane equation: nx*x + ny*y + nz*z + d = 0
    double d = -(nx * b.x + ny * b.y + nz * b.z);

    // Ray-plane intersection: t = -(N.P1 + d) / (N.D)
    double denom = nx * ndx + ny * ndy + nz * ndz;
    
    // Check if moving towards the face (denom < 0) or generic intersection
    if (fabs(denom) > 1e-6) {
      double t = -(nx * tip1x + ny * tip1y + nz * tip1z + d) / denom;
      if (t >= 0 && t <= len) {
        // Intersection point (World coords of hit)
        double ix = tip1x + t * ndx;
        double iy = tip1y + t * ndy;
        double iz = tip1z + t * ndz;

        // Check distance from center
        double dist = sqrt(pow(ix - b.x, 2) + pow(iy - b.y, 2) + pow(iz - b.z, 2));
        if (dist <= b.radius) {
          // Hit! Calculate score
          double ringWidth = b.radius / b.rings;
          int ringIndex = (int)(dist / ringWidth);
          if (ringIndex >= b.rings) ringIndex = b.rings - 1;

          // Scoring Logic:
          // Fewer rings = harder target = more points per ring.
          // Base multiplier scales inversely with ring count.
          // Max rings is 6 (from getBullseye).
          // Multiplier = 6.0 / b.rings
          double multiplier = 6.0 / b.rings;
          
          int score = (int)((b.rings - ringIndex) * 10 * multiplier);
          if (ringIndex == 0) score += (int)(20 * multiplier); // Scale bonus too

          // STICK THE ARROW
          arrow->stuck = 1;
          arrow->stuckTargetIndex = i;
          arrow->active = 1; // Keep active so it gets drawn, but physics will skip it
          
          // Calculate relative position/rotation
          // We need to transform the intersection point (ix, iy, iz) into the bullseye's local space
          // Local space basis: X=Forward, Y=Up, Z=Normal
          // P_local = [X Y Z]^T * (P_world - Origin)
          
          double relX = ix - b.x;
          double relY = iy - b.y;
          double relZ = iz - b.z;
          
          // Project onto basis vectors
          // Note: This is the relative position of the TIP
          double tipRelX = relX * fx + relY * fy + relZ * fz;
          double tipRelY = relX * ux + relY * uy + relZ * uz;
          double tipRelZ = relX * nx + relY * ny + relZ * nz;
          
          // We want to store the relative position of the ARROW ORIGIN (Tail)
          // Arrow Origin = Tip - Dir * arrowLen
          // We need Dir in local space too
          double localDx = arrow->dx * fx + arrow->dy * fy + arrow->dz * fz;
          double localDy = arrow->dx * ux + arrow->dy * uy + arrow->dz * uz;
          double localDz = arrow->dx * nx + arrow->dy * ny + arrow->dz * nz;
          
          arrow->stuckRelX = tipRelX - localDx * arrowLen;
          arrow->stuckRelY = tipRelY - localDy * arrowLen;
          arrow->stuckRelZ = tipRelZ - localDz * arrowLen;
          
          arrow->stuckRelDx = localDx;
          arrow->stuckRelDy = localDy;
          arrow->stuckRelDz = localDz;
          
          // Snap arrow position to intersection point exactly (offset by length)
          arrow->x = ix - arrow->dx * arrowLen;
          arrow->y = iy - arrow->dy * arrowLen;
          arrow->z = iz - arrow->dz * arrowLen;

          return score;
        }
      }
    }
    i++;
  }
  return 0;
}
