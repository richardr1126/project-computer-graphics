/*
 *  Bullseye object - header file
 *  Defines bullseye structure and drawing functions
 */

#ifndef OBJECTS_BULLSEYE_H
#define OBJECTS_BULLSEYE_H

/*
 *  Bullseye description for passing parameters around
 */
typedef struct {
  /* world position */
  double x, y, z;
  /* local X direction vector (orientation) */
  double dx, dy, dz;
  /* local Y up vector (orientation) */
  double ux, uy, uz;
  /* geometry */
  double radius; /* outer radius */
  int rings;     /* number of rings */
  /* color for alternating rings */
  double r, g, b;
} Bullseye;

/*
 *  Function prototypes
 */

/*
 *  Draw a single bullseye from a Bullseye struct
 *  @param b pointer to Bullseye structure
 *  @param texture OpenGL texture ID (0 for no texture)
 */
void drawBullseye(const Bullseye *b, unsigned int texture);

/*
 *  Draw the scene with multiple bullseye targets
 *  @param zh animation angle in degrees for bullseye motion
 *  @param texture OpenGL texture ID for bullseyes
 */
void drawBullseyeScene(double zh, unsigned int texture);

/*
 *  Get a bullseye definition by index
 *  @param index index of the bullseye (0 to N-1)
 *  @param zh animation angle
 *  @param b pointer to Bullseye structure to fill
 *  @return 1 if index is valid, 0 otherwise
 */
int getBullseye(int index, double zh, Bullseye *b);

/*
 *  Check collision between arrow and bullseyes
 *  @param arrow pointer to Arrow structure (modified if stuck)
 *  @param zh animation angle
 *  @return score (0 if no hit)
 */
int checkBullseyeCollision(void *arrow, double zh); // void* to avoid circular dependency if arrow.h not included

#endif
