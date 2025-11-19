/*
 *  View module - header file
 *  Contains view and projection functions
 */
#ifndef VIEW_H
#define VIEW_H

/*
 *  Function prototypes
 */
void Project(int mode, int fov, double asp, double dim);
void setViewMode(int mode, double th, double ph, double dim, double px,
                 double py, double pz);

/*
 * First-person helpers for smooth input
 */
void fpUpdateMove(int th, int kForward, int kBackward, int kLeft, int kRight,
                  double speed, double dt, double *px, double *pz);

#endif