#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

// GLEW _MUST_ be included first
#ifdef USEGLEW
#include <GL/glew.h>
#endif
//  Get all GL prototypes
#define GL_GLEXT_PROTOTYPES
//  Select SDL, SDL2, GLFW or GLUT
#if defined(SDL2)
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#elif defined(SDL)
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#elif defined(GLFW)
#include <GLFW/glfw3.h>
#elif defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
//  Make sure GLU and GL are included
#ifdef __APPLE__
#include <OpenGL/glu.h>
#include <OpenGL/gl.h>
// Tell Xcode IDE to not gripe about OpenGL deprecation
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#include <GL/glu.h>
#include <GL/gl.h>
#endif

//  cos and sin in degrees
#define Cos(th) cos(3.14159265/180*(th))
#define Sin(th) sin(3.14159265/180*(th))

#ifdef __cplusplus
extern "C" {
#endif

//  Macros for printf-like functions to enable format string checking
#ifdef __GNUC__
void Print(const char* format , ...) __attribute__ ((format(printf,1,2)));
void Fatal(const char* format , ...) __attribute__ ((format(printf,1,2))) __attribute__ ((noreturn));
#else
void Print(const char* format , ...);
void Fatal(const char* format , ...);
#endif
// Utilities
void ErrCheck(const char* where);
unsigned int LoadTexBMP(const char* file);

// Math helpers
double Vec3Length(double x, double y, double z);
void Vec3Normalize(double* x, double* y, double* z);
void Vec3Cross(double ax, double ay, double az, double bx, double by, double bz,
               double* rx, double* ry, double* rz);
void DirectionFromAngles(double th, double ph,
                         double* dx, double* dy, double* dz);
double Rand01(unsigned int seed);


#ifdef __cplusplus
}
#endif

#endif
