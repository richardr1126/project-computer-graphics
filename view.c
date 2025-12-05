/*
 *  View module - implementation file
 *  Contains view and projection functions
 */
#include "view.h"
#include "utils.h"

/*
 *  Set projection matrix (perspective)
 *  @param mode projection mode
 *  @param fov field of view
 *  @param asp aspect ratio
 *  @param dim distance to near plane
 */
void Project(int mode, int fov, double asp, double dim) {
  //  Tell OpenGL we want to manipulate the projection matrix
  glMatrixMode(GL_PROJECTION);
  //  Undo previous transformations
  glLoadIdentity();
  if (mode == 1)
    //  Perspective transformation - extend far plane to see entire scene
    gluPerspective(fov, asp, dim / 16, 16 * dim);
  else if (mode == 2) {
    // First-person: use a much closer near plane to avoid clipping when near objects
    /* (helped by AI) */
    const double near = 0.05;
    const double far = 16 * dim;
    gluPerspective(fov, asp, near, far);
  }
  //  Switch to manipulating the model matrix
  glMatrixMode(GL_MODELVIEW);
  //  Undo previous transformations
  glLoadIdentity();
}

/*
 *  Set the current view based on the projection mode and angles
 *  @param mode projection mode
 *  @param th azimuthal angle
 *  @param ph polar angle
 *  @param dim distance to near plane
 *  @param px x position
 *  @param py y position
 *  @param pz z position
 */
void setViewMode(int mode, double th, double ph, double dim, double px,
                 double py, double pz) {
  //  Set camera/view depending on projection mode
  if (mode == 1) {
    // Perspective - position the eye using spherical angles
    double Ex = -2 * dim * Sin(th) * Cos(ph);
    double Ey = +2 * dim * Sin(ph);
    double Ez = +2 * dim * Cos(th) * Cos(ph);
    gluLookAt(Ex, Ey, Ez, 0, 0, 0, 0, Cos(ph), 0);
  } else if (mode == 2) {
    // First-person: look from (px,py,pz) along yaw/pitch defined by th/ph
    // Match orbit convention so th=0 looks toward -Z.
    double dirX, dirY, dirZ;
    DirectionFromAngles(th, ph, &dirX, &dirY, &dirZ);
    gluLookAt(px, py, pz, px + dirX, py + dirY, pz + dirZ, 0, 1, 0);
  }
}

/*
 *  Smooth first-person movement: updates px/pz using WASD state and dt
 *  Normalizes diagonal motion to keep constant speed
 *  @param th azimuthal angle
 *  @param kForward forward key state
 *  @param kBackward backward key state
 *  @param kLeft left key state
 *  @param kRight right key state
 *  @param speed movement speed
 *  @param dt time step
 *  @param px x position
 *  @param pz z position
 */
void fpUpdateMove(int th, int kForward, int kBackward, int kLeft, int kRight,
                  double speed, double dt, double *px, double *pz) {
  int mvF = (kForward ? 1 : 0) - (kBackward ? 1 : 0);
  int mvR = (kRight ? 1 : 0) - (kLeft ? 1 : 0);
  if (!(mvF || mvR))
    return;

  double fx = Sin(th);
  double fz = -Cos(th);
  double rx = Cos(th);
  double rz = Sin(th);
  double vx = mvF * fx + mvR * rx;
  double vz = mvF * fz + mvR * rz;
  double len = sqrt(vx * vx + vz * vz);
  if (len > 1e-6) {
    vx /= len;
    vz /= len;
    *px += vx * speed * dt;
    *pz += vz * speed * dt;
  }
}