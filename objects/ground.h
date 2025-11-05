/*
 *  Ground terrain object - header file
 *  Defines terrain drawing functions
 */

#ifndef OBJECTS_GROUND_H
#define OBJECTS_GROUND_H

/*
 *  Draw ground terrain with varied height
 *  steepness: terrain height multiplier
 *  size: ground extends from -size to +size in X and Z
 *  groundY: base height offset in Y direction
 *  texture: OpenGL texture ID (0 for no texture)
 *  showNormals: whether to draw normal vectors
 */
void drawGround(double steepness, double size, double groundY, unsigned int texture, int showNormals);

#endif
