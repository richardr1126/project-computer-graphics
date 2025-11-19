/*
 *  Lighting objects - header file
 *  Defines light ball drawing functions
 */

#ifndef OBJECTS_LIGHTING_H
#define OBJECTS_LIGHTING_H

/*
 *  Draw a small sphere to represent the light (unlit so it appears emissive)
 *  x, y, z: position of the light ball
 *  r: radius of the light ball
 *  isDay: 1 for sun (day), 0 for moon (night)
 */
void drawLightBall(double x, double y, double z, double r, int isDay);

/*
 *  Draw the sky background with day/night cycle
 *  dayNightCycle: 0.0 to 1.0, where 0.0 and 1.0 are noon, 0.5 is midnight
 */
void drawSky(double dayNightCycle);

#endif
