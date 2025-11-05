/*
 *  Recursive tree object - header file
 */

#ifndef OBJECTS_TREE_H
#define OBJECTS_TREE_H

/*
 * Draw a forest of recursive trees around the bullseye scene
 * anim: animation parameter (e.g., sway angle in degrees)
 * showNormals: toggle to draw debug normals
 * texture: OpenGL texture ID for bark
 */
void drawTreeScene(double anim, int showNormals, unsigned int texture);

#endif
