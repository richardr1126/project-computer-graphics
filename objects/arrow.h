#ifndef ARROW_H
#define ARROW_H

/*
 *  Arrow structure
 */
typedef struct {
  double x, y, z;    // Position
  double prevX, prevY, prevZ; // Previous position (for collision detection)
  double dx, dy, dz; // Direction
  double vx, vy, vz; // Velocity
  double scale;      // Scale factor (optional, default 1.0)
  int active;        // 1 if arrow is flying, 0 otherwise
  // Sticky state
  int stuck;              // 1 if stuck to a target
  int stuckTargetIndex;   // Index of the target
  double stuckRelX, stuckRelY, stuckRelZ;    // Relative position to target center
  double stuckRelDx, stuckRelDy, stuckRelDz; // Relative direction
} Arrow;

/*
 *  Draw an arrow from an Arrow struct
 *  @param arrow pointer to Arrow structure
 */
void drawArrow(const Arrow *arrow);

/*
 *  Update arrow physics
 *  @param arrow pointer to Arrow structure
 *  @param dt time delta in seconds
 */
void updateArrow(Arrow *arrow, double dt);

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
                double ph, double speed);

/*
 *  Update arrow position when stuck to a target
 *  @param arrow pointer to Arrow structure
 *  @param zh animation angle of targets
 */
void updateStuckArrow(Arrow *arrow, double zh);

#endif
