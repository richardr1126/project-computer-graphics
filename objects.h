/*
 *  Objects module - header file
 *  Contains drawing functions for 3D objects
 */

#ifndef OBJECTS_H
#define OBJECTS_H

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
void drawAxes(double len);

void drawBullseyeScene(double zh, int showNormals, unsigned int texture);

void drawLightBall(double x, double y, double z, double r);

void drawGround(double steepness, double size, double groundY, unsigned int texture, int showNormals);

#endif