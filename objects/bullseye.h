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
    double radius;   /* outer radius */
    int    rings;    /* number of rings */
    /* color for alternating rings */
    double r, g, b;
} Bullseye;

/*
 *  Function prototypes
 */

/*
 *  Draw a single bullseye from a Bullseye struct
 *  b: pointer to Bullseye structure
 *  texture: OpenGL texture ID (0 for no texture)
 */
void drawBullseye(const Bullseye* b, unsigned int texture);

/*
 *  Draw the scene with multiple bullseye targets
 *  zh: animation angle in degrees for bullseye motion
 *  showNormals: whether to draw normal vectors
 *  texture: OpenGL texture ID for bullseyes
 */
void drawBullseyeScene(double zh, int showNormals, unsigned int texture);

#endif
