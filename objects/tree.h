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
  double baseLength; /* initial trunk length */
  double baseRadius; /* initial trunk radius */
  int depth;         /* recursion depth */
  /* textures */
  unsigned int barkTexture;
  unsigned int leafTexture;
  /* animation and rendering */
  double anim;     /* animation parameter (sway angle in degrees) */
  /* seeding for procedural variation */
  unsigned int seed;
} Tree;

/*
 *  Function prototypes
 */

/*
 *  Draw a single tree from a Tree struct
 *  @param t pointer to Tree structure
 *  @param leavesOnly whether to draw only leaves (for transparent pass)
 */
void drawTree(const Tree *t, int leavesOnly);

/*
 *  Draw the scene with a forest of trees around the bullseye scene
 *  @param anim animation parameter (e.g., sway angle in degrees)
 *  @param barkTexture OpenGL texture ID for bark
 *  @param leafTexture OpenGL texture ID for leaves (0 = draw only trunks/branches)
 */
void drawTreeScene(double anim, unsigned int barkTexture,
                   unsigned int leafTexture);

/*
 *  Draw only the leaves for all trees (for transparent pass)
 *  @param anim animation parameter (e.g., sway angle in degrees)
 *  @param leafTexture OpenGL texture ID for leaves
 */
void drawTreeLeaves(double anim, unsigned int leafTexture);

#endif
