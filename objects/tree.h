/*
 *  Recursive tree object - header file
 *  Defines tree structure and drawing functions
 */

#ifndef OBJECTS_TREE_H
#define OBJECTS_TREE_H

/*
 *  Tree description for passing parameters around
 */
typedef struct {
    /* world position */
    double x, y, z;
    /* geometry */
    double baseLength;  /* initial trunk length */
    double baseRadius;  /* initial trunk radius */
    int depth;          /* recursion depth */
    /* textures */
    unsigned int barkTexture;
    unsigned int leafTexture;
    /* animation and rendering */
    double anim;        /* animation parameter (sway angle in degrees) */
    int showNormals;    /* toggle to draw debug normals */
    /* seeding for procedural variation */
    unsigned int seed;
} Tree;

/*
 *  Function prototypes
 */

/*
 *  Draw a single tree from a Tree struct
 *  t: pointer to Tree structure
 */
void drawTree(const Tree* t, int leavesOnly);

/*
 *  Draw the scene with a forest of trees around the bullseye scene
 *  anim: animation parameter (e.g., sway angle in degrees)
 *  showNormals: toggle to draw debug normals
 *  barkTexture: OpenGL texture ID for bark
 *  leafTexture: OpenGL texture ID for leaves (0 = draw only trunks/branches)
 */
void drawTreeScene(double anim, int showNormals, unsigned int barkTexture, unsigned int leafTexture);

/*
 *  Draw only the leaves for all trees (for transparent pass)
 *  anim: animation parameter (e.g., sway angle in degrees)
 *  leafTexture: OpenGL texture ID for leaves
 */
void drawTreeLeaves(double anim, unsigned int leafTexture);

#endif
