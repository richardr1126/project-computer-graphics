/*
 *  Ground terrain object - header file
 *  Defines terrain drawing functions
 */

#ifndef OBJECTS_GROUND_H
#define OBJECTS_GROUND_H

/*
 *  Draw ground terrain with varied height
 *  @param steepness terrain height multiplier
 *  @param size ground extends from -size to +size in X and Z
 *  @param groundY base height offset in Y direction
 *  @param texture OpenGL texture ID (0 for no texture)
 */
void drawGround(double steepness, double size, double groundY,
                unsigned int texture);

/*
 *  Draw a circular mountain ring (bowl-like) surrounding the ground island
 *  @param innerR inner radius (should match ground size for a seamless join)
 *  @param outerR outer radius of the mountains
 *  @param baseY base height offset in Y direction (same as groundY)
 *  @param texture OpenGL texture ID for mountains (e.g., ground2.bmp)
 *  @param heightScale vertical scale of the mountains (higher => taller mountains)
 */
void drawMountainRing(double innerR, double outerR, double baseY,
                      unsigned int texture, double heightScale);

#endif
