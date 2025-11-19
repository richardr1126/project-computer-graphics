#ifndef ARROW_H
#define ARROW_H

/*
 *  Arrow structure
 */
typedef struct {
  double x, y, z;    // Position
  double dx, dy, dz; // Direction
  double vx, vy, vz; // Velocity
  double scale;      // Scale factor (optional, default 1.0)
  int active;        // 1 if arrow is flying, 0 otherwise
} Arrow;

/*
 *  Draw an arrow from an Arrow struct
 *  arrow: pointer to Arrow structure
 *  showNormals: 1 to draw normals, 0 otherwise
 */
void drawArrow(const Arrow *arrow, int showNormals);

/*
 *  Update arrow physics
 *  arrow: pointer to Arrow structure
 *  dt: time delta in seconds
 */
void updateArrow(Arrow *arrow, double dt);

/*
 *  Shoot arrow from position with angle
 *  arrow: pointer to Arrow structure
 *  x, y, z: starting position
 *  th, ph: view angles (degrees)
 *  speed: initial speed
 */
void shootArrow(Arrow *arrow, double x, double y, double z, double th,
                double ph, double speed);

#endif
