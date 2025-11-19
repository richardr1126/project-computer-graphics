#ifndef ARROW_H
#define ARROW_H

/*
 *  Arrow structure
 */
typedef struct {
    double x, y, z;      // Position
    double dx, dy, dz;   // Direction
    double scale;        // Scale factor (optional, default 1.0)
} Arrow;

/*
 *  Draw an arrow from an Arrow struct
 *  arrow: pointer to Arrow structure
 *  showNormals: 1 to draw normals, 0 otherwise
 */
void drawArrow(const Arrow* arrow, int showNormals);

#endif
