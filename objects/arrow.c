#include "arrow.h"
#include "bullseye.h"
#include "../utils.h"

/*
 *  Draw a cylinder
 *  @param r radius
 *  @param h height
 */
static void Cylinder(double r, double h) {
  const int d = 15;
  glBegin(GL_QUAD_STRIP);
  for (int th = 0; th <= 360; th += d) {
    double x = r * Cos(th);
    double y = r * Sin(th);
    glNormal3d(Cos(th), Sin(th), 0);
    glVertex3d(x, y, 0);
    glVertex3d(x, y, h);
  }
  glEnd();

  // Bottom cap
  glBegin(GL_TRIANGLE_FAN);
  glNormal3d(0, 0, -1);
  glVertex3d(0, 0, 0);
  for (int th = 360; th >= 0; th -= d) {
    double x = r * Cos(th);
    double y = r * Sin(th);
    glVertex3d(x, y, 0);
  }
  glEnd();
}

/*
 *  Draw a cone
 *  @param r radius
 *  @param h height
 */
static void Cone(double r, double h) {
  const int d = 15;
  glBegin(GL_TRIANGLE_FAN);
  glNormal3d(0, 0, 1); // Tip normal approximation
  glVertex3d(0, 0, h);
  for (int th = 0; th <= 360; th += d) {
    double x = r * Cos(th);
    double y = r * Sin(th);
    // Side normals
    double len = sqrt(r * r + h * h);
    double nx = h / len * Cos(th);
    double ny = h / len * Sin(th);
    double nz = r / len;

    glNormal3d(nx, ny, nz);
    glVertex3d(x, y, 0);
  }
  glEnd();

  // Base cap
  glBegin(GL_TRIANGLE_FAN);
  glNormal3d(0, 0, -1);
  glVertex3d(0, 0, 0);
  for (int th = 360; th >= 0; th -= d) {
    double x = r * Cos(th);
    double y = r * Sin(th);
    glVertex3d(x, y, 0);
  }
  glEnd();
}

/*
 *  Draw an arrow from an Arrow struct
 *  @param arrow pointer to Arrow structure
 */
void drawArrow(const Arrow *arrow) {
  if (!arrow || !arrow->active) return;

  double x = arrow->x;
  double y = arrow->y;
  double z = arrow->z;
  double dx = arrow->dx;
  double dy = arrow->dy;
  double dz = arrow->dz;

  // Calculate rotation angles to align the arrow with (dx,dy,dz)
  // Default arrow points along +Z axis
  double length = sqrt(dx * dx + dy * dy + dz * dz);
  if (length < 1e-6) return;

  // Normalize direction
  // Calculate yaw (rotation around Y) and pitch (rotation around X)
  // Arrow is initially pointing along +Z.
  double yaw = atan2(dx, dz) * 180.0 / M_PI;
  double pitch = -asin(dy / length) * 180.0 / M_PI;

  glPushMatrix();

  // Translate to position
  glTranslated(x, y, z);

  // Rotate to direction
  glRotated(yaw, 0, 1, 0);
  glRotated(pitch, 1, 0, 0);

  // Scale if needed
  if (arrow->scale > 0.0) glScaled(arrow->scale, arrow->scale, arrow->scale);

  // Draw Shaft (Cylinder)
  // Cylinder starts at Z=0 and goes to Z=length
  glColor3f(0.6, 0.4, 0.2); // Wood color
  double shaftLength = 3.0;
  double shaftRadius = 0.05;
  Cylinder(shaftRadius, shaftLength);

  // Draw Tip (Cone)
  glPushMatrix();
  glTranslated(0, 0, shaftLength);
  glColor3f(0.5, 0.5, 0.5); // Metal color
  double tipLength = 0.5;
  double tipRadius = 0.1;
  Cone(tipRadius, tipLength);
  glPopMatrix();

  // Draw Fletching (Feathers)
  // 3 feathers at 120 degrees
  glColor3f(1.0, 0.0, 0.0); // Red feathers
  double fletchLength = 0.8;
  double fletchWidth = 0.2;

  for (int i = 0; i < 3; i++) {
    glPushMatrix();
    glRotated(120 * i, 0, 0, 1);
    glBegin(GL_TRIANGLES);
    // Simple triangle fletching
    glNormal3d(1, 0, 0); // Approximate normal
    glVertex3d(0, shaftRadius, 0);
    glVertex3d(0, shaftRadius + fletchWidth, 0.2);
    glVertex3d(0, shaftRadius, fletchLength);

    // Double sided?
    glNormal3d(-1, 0, 0);
    glVertex3d(0, shaftRadius, fletchLength);
    glVertex3d(0, shaftRadius + fletchWidth, 0.2);
    glVertex3d(0, shaftRadius, 0);
    glEnd();
    glPopMatrix();
  }

  glPopMatrix();
}

/*
 *  Update arrow physics
 *  @param arrow pointer to Arrow structure
 *  @param dt time delta in seconds
 */
void updateArrow(Arrow *arrow, double dt) {
  if (!arrow || !arrow->active) return;

  // Gravity
  const double g = 9.8; // m/s^2
  arrow->vy -= g * dt;

  // Update position
  arrow->prevX = arrow->x;
  arrow->prevY = arrow->y;
  arrow->prevZ = arrow->z;
  arrow->x += arrow->vx * dt;
  arrow->y += arrow->vy * dt;
  arrow->z += arrow->vz * dt;

  // Update direction to match velocity
  double speed = sqrt(arrow->vx * arrow->vx + arrow->vy * arrow->vy +
                      arrow->vz * arrow->vz);
  if (speed > 0.001) {
    arrow->dx = arrow->vx / speed;
    arrow->dy = arrow->vy / speed;
    arrow->dz = arrow->vz / speed;
  }
}

/*
 *  Shoot arrow from position with angle
 *  @param arrow pointer to Arrow structure
 *  @param x starting position x
 *  @param y starting position y
 *  @param z starting position z
 *  @param th view angle theta (degrees)
 *  @param ph view angle phi (degrees)
 *  @param speed initial speed
 */
void shootArrow(Arrow *arrow, double x, double y, double z, double th,
                double ph, double speed) {
  if (!arrow) return;

  arrow->active = 1;
  arrow->active = 1;
  arrow->x = x;
  arrow->y = y;
  arrow->z = z;
  arrow->prevX = x;
  arrow->prevY = y;
  arrow->prevZ = z;
  arrow->scale = 1.0;

  // Convert view angles to direction vector
  DirectionFromAngles(th, ph, &arrow->dx, &arrow->dy, &arrow->dz);

  arrow->vx = arrow->dx * speed;
  arrow->vy = arrow->dy * speed;
  arrow->vz = arrow->dz * speed;
}

/*
 *  Update arrow position when stuck to a target
 *  @param arrow pointer to Arrow structure
 *  @param zh animation angle of targets
 */
void updateStuckArrow(Arrow *arrow, double zh) {
  if (!arrow || !arrow->stuck) return;

  // Update position based on target
  Bullseye b;
  // We need to include bullseye.h for this
  // Assuming arrow.c includes bullseye.h now
  if (getBullseye(arrow->stuckTargetIndex, zh, &b)) {
    // Reconstruct world position from relative
    // P_world = Origin + P_local * Basis
    // Basis: X=Forward(dx,dy,dz), Y=Up(ux,uy,uz), Z=Normal(nx,ny,nz)
    
    double fx = b.dx, fy = b.dy, fz = b.dz;
    double ux = b.ux, uy = b.uy, uz = b.uz;
    double nx, ny, nz;
    Vec3Cross(fx, fy, fz, ux, uy, uz, &nx, &ny, &nz);
    Vec3Normalize(&nx, &ny, &nz);
    
    double rx = arrow->stuckRelX;
    double ry = arrow->stuckRelY;
    double rz = arrow->stuckRelZ;
    
    arrow->x = b.x + rx * fx + ry * ux + rz * nx;
    arrow->y = b.y + rx * fy + ry * uy + rz * ny;
    arrow->z = b.z + rx * fz + ry * uz + rz * nz;
    
    // Update direction too
    double rdx = arrow->stuckRelDx;
    double rdy = arrow->stuckRelDy;
    double rdz = arrow->stuckRelDz;
    
    arrow->dx = rdx * fx + rdy * ux + rdz * nx;
    arrow->dy = rdx * fy + rdy * uy + rdz * ny;
    arrow->dz = rdx * fz + rdy * uz + rdz * nz;
  }
}
