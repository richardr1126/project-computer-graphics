/*
 *  Lighting objects - implementation file
 *  Contains drawing functions for light representation
 */

#include "lighting.h"
#include "../utils.h"

/*
 *  Vertex on a unit sphere given angles (degrees)
 *  Original Author: Willem A. (Vlakkies) Schreuder
 *  @param th angle in degrees
 *  @param ph angle in degrees
 */
static void SphereVertex(double th, double ph) {
  double x = Sin(th) * Cos(ph);
  double y = Cos(th) * Cos(ph);
  double z = Sin(ph);
  glNormal3d(x, y, z);
  // Texture coordinates: spherical mapping
  glTexCoord2d(th / 360.0, (ph + 90.0) / 180.0);
  glVertex3d(x, y, z);
}

/*
 *  Draw a lit sphere using latitude-longitude quads
 *  @param x x position
 *  @param y y position
 *  @param z z position
 *  @param r radius
 *  @param inc angular increment
 */
static void drawBall(double x, double y, double z, double r, int inc) {
  if (inc < 1)
    inc = 1;
  // Save transform and move/scale
  glPushMatrix();
  glTranslated(x, y, z);
  glScaled(r, r, r);
  // Bands of latitude
  for (int ph = -90; ph < 90; ph += inc) {
    glBegin(GL_QUAD_STRIP);
    for (int th = 0; th <= 360; th += 2 * inc) {
      SphereVertex(th, ph);
      SphereVertex(th, ph + inc);
    }
    glEnd();
  }
  glPopMatrix();
}

/*
 *  Draw a small sphere to represent the light (unlit so it appears emissive)
 *  @param x x position
 *  @param y y position
 *  @param z z position
 *  @param r radius
 *  @param isDay 1 for sun (day), 0 for moon (night)
 */
void drawLightBall(double x, double y, double z, double r, int isDay) {
  glPushMatrix();
  // Draw unlit so it appears emissive and not affected by scene lighting
  GLboolean wasLit = glIsEnabled(GL_LIGHTING);
  glDisable(GL_LIGHTING);

  if (isDay) {
    // Sun: bright yellow/white
    glColor3f(1.0, 1.0, 0.7);
  } else {
    // Moon: dimmer blue/white
    glColor3f(0.8, 0.8, 1.0);
  }
  drawBall(x, y, z, r, 3);

  if (wasLit)
    glEnable(GL_LIGHTING);
  glPopMatrix();
}

/*
 *  Draw the sky background with day/night cycle
 *  @param dayNightCycle 0.0 to 1.0, where 0.0 and 1.0 are noon, 0.5 is midnight
 */
void drawSky(double dayNightCycle) {
  // Save current state
  GLboolean wasLit = glIsEnabled(GL_LIGHTING);
  GLboolean wasDepthTest = glIsEnabled(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);

  // Calculate sky colors based on time of day
  // dayNightCycle: 0.0-0.25 = morning, 0.25-0.5 = night, 0.5-0.75 = morning, 0.75-1.0 = day
  double dayFactor;
  
  // Convert cycle to a smooth curve where 0 and 1 are day, 0.5 is night
  // Use cosine for smooth transitions: cos(0) = 1 (day), cos(PI) = -1 (night)
  dayFactor = (Cos(dayNightCycle * 360.0) + 1.0) / 2.0; // Range 0 to 1

  // Top color (sky zenith)
  double topR, topG, topB;
  // Bottom color (horizon)
  double botR, botG, botB;

  // Day sky: bright blue top, lighter blue bottom
  double dayTopR = 0.4, dayTopG = 0.6, dayTopB = 0.9;
  double dayBotR = 0.7, dayBotG = 0.85, dayBotB = 1.0;

  // Night sky: dark blue top, slightly lighter bottom
  double nightTopR = 0.05, nightTopG = 0.05, nightTopB = 0.15;
  double nightBotR = 0.1, nightBotG = 0.1, nightBotB = 0.25;

  // Interpolate between day and night colors
  topR = nightTopR + dayFactor * (dayTopR - nightTopR);
  topG = nightTopG + dayFactor * (dayTopG - nightTopG);
  topB = nightTopB + dayFactor * (dayTopB - nightTopB);

  botR = nightBotR + dayFactor * (dayBotR - nightBotR);
  botG = nightBotG + dayFactor * (dayBotG - nightBotG);
  botB = nightBotB + dayFactor * (dayBotB - nightBotB);

  // Draw sky as a large quad with gradient
  glPushMatrix();
  glLoadIdentity();
  
  // Set up projection to cover entire screen
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(-1, 1, -1, 1, -1, 1);
  glMatrixMode(GL_MODELVIEW);

  // Draw gradient quad
  glBegin(GL_QUADS);
  // Top vertices (sky)
  glColor3f(topR, topG, topB);
  glVertex3f(-1, 1, -0.999);
  glVertex3f(1, 1, -0.999);
  // Bottom vertices (horizon)
  glColor3f(botR, botG, botB);
  glVertex3f(1, -1, -0.999);
  glVertex3f(-1, -1, -0.999);
  glEnd();

  // Restore projection
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  // Restore state
  if (wasLit)
    glEnable(GL_LIGHTING);
  if (wasDepthTest)
    glEnable(GL_DEPTH_TEST);
}
