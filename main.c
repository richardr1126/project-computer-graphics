/*
 *  Final Project: Richard Roberson
 *
 *  Key Bindings:
 *  View Controls:
 *    TAB    Toggle view modes: Perspective (orbit) <-> First-Person
 *    +/-    Change field of view (perspective modes)
 *    [/]    Zoom in/out (orbit modes only)
 *    0      Reset view (camera position/angles, FOV)
 *    g/G    Toggle axes display
 *    h/H    Cycle HUD modes (0=hint only, 1=controls, 2=all)
 *    ESC    Exit
 *  Camera Controls:
 *    Left-click drag Look around (first-person mode)
 *    Right-click     Hold to charge, release to shoot arrow
 *    arrows          Look around (perspective orbit mode)
 *    w/s             Move forward/backward (first-person mode only)
 *    a/d             Strafe left/right (first-person mode only)
 *  Lighting Controls:
 *    l/L    Toggle lighting on/off
 *    1/2    Raise/lower light height
 *    3/4    Increase/decrease light distance
 *  Sky/Time Controls (light rotation tied to day/night cycle):
 *    5      Pause/resume day/night cycle and light rotation
 *    6/7    Increase/decrease cycle speed
 *    9      Manual time step forward (when paused)
 *  Other Controls:
 *    o/O    Toggle texture filtering optimizations (mipmaps + anisotropy)
 *    f/F    Toggle distance fog
 *    p/P    Pause/resume bullseye motion
 *    n/N    Toggle normals debug lines
 */
//  Include custom modules
#include "objects/arrow.h"
#include "objects/axes.h"
#include "objects/bullseye.h"
#include "objects/ground.h"
#include "objects/lighting.h"
#include "objects/tree.h"
#include "utils.h"
#include "view.h"

//  Global state variables
// View parameters
double th = 0.0;   //  Azimuth of view angle (degrees)
double ph = 0.0;   //  Elevation of view angle (degrees)
int axes = 0;      //  Display axes
int mode = 2;      //  View mode: 1=Perspective (orbit), 2=First-Person
int fov = 55;      //  Field of view for perspective
double asp = 1;    //  Aspect ratio
double dim = 25.0; //  World size for projection (increased to see full scene)
int showHUD = 2;   //  HUD mode: 0=hint only, 1=controls only, 2=all
//  First-person camera
double px = 0, py = 0, pz = 30;     // Position of the camera in world coords
double moveStep = 7.0;              // Movement speed (units per second)
int kW = 0, kA = 0, kS = 0, kD = 0; // WASD movement keys
// Mouse look state (first-person)
int mouseLook = 0;              // 1 while left or right button held for looking
int leftMouseDown = 0;
int rightMouseDown = 0;
int lastX = 0, lastY = 0;       // last mouse position
double mouseSensitivity = 0.15; // degrees per pixel (lower for smoother feel)
// Bullseye motion
double zhTargets = 0;     // Animation angle for bullseye motion (degrees)
int moveTargets = 1;      // Toggle bullseye motion
double targetRate = 90.0; // default target motion speed (degrees per second)
// Trees animation (wind sway)
double zhTrees = 0; // Animation angle for tree sway (degrees)
// Arrow state
Arrow arrow = {0, 5, 0, 1, 0, 0, 0, 0, 0, 1.0, 0}; // Initial static arrow
double chargeStartTime = 0; // Time when right click started
//  Lighting
int light = 1;           // Lighting toggle
double ylight = 12.0;     // Elevation of the light
double ldist = 24.0;     // Light distance from origin in XZ plane
//  Fog
int fog = 1;             // Fog toggle (1=enabled, 0=disabled)
//  Day/Night Cycle
double dayNightCycle = 0.0;   // 0.0-1.0: 0 and 1 are noon, 0.5 is midnight
double cycleRate = 0.05;      // cycle speed (cycles per second) = 20 second full cycle
int moveCycle = 1;            // Toggle day/night cycle motion
//  Debug helpers
int showNormals = 0; //  Toggle drawing of normal vectors
//  Textures
int textureOptimizations = 1; //  Texture filtering mode: 1=optimized, 0=basic
int anisoSupported = 0;
float maxAniso = 1.0f;
unsigned int groundTexture = 0;   // Ground texture ID
unsigned int mountainTexture = 0; // Mountain ring texture ID
unsigned int woodTexture = 0;     // Wood texture ID for bullseyes
unsigned int barkTexture = 0;     // Bark texture ID for trees
unsigned int leafTexture = 0;     // Leaf texture ID for tree foliage
// FPS tracking
double fps = 0.0;         // Current frames per second
int frameCount = 0;       // Frame counter for FPS calculation
double lastFPSTime = 0.0; // Last time FPS was calculated

// Cache anisotropic filter support once we have a GL context
void detectAnisoSupport() {
  const char *ext = (const char *)glGetString(GL_EXTENSIONS);
  if (ext && strstr(ext, "GL_EXT_texture_filter_anisotropic")) {
    anisoSupported = 1;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
  }
}

// Apply the current filtering mode to a texture (optimized vs basic)
void applyTextureFiltering(unsigned int texture) {
  if (!texture) return;
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  if (textureOptimizations) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    if (anisoSupported) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    if (anisoSupported) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
  }
}

/*
 *  Draw HUD with controls and status information
 *  Mode 0: Just hint to press H
 *  Mode 1: Controls only
 *  Mode 2: Everything (status + controls)
 */
void drawHUD() {
  int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
  glColor3f(1, 1, 1);


  // Mode 0: Just show hint
  if (showHUD == 0) {
    int yTop = windowHeight - 15;
    glWindowPos2i(5, yTop);
    Print("H) HUD");
    return;
  }

  // Mode 1 or 2: Show controls (at top of screen)
  int yTop = windowHeight - 15;
  // Controls section header
  glWindowPos2i(5, yTop);
  Print("Controls:");
  // View Controls
  yTop -= 15;
  glWindowPos2i(5, yTop);
  if (mode == 1)
    Print("  View: TAB)Modes  +/-)FOV  [/])Zoom  0)Reset  G)Axes  H)HUD  "
          "ESC)Exit");
  else
    Print("  View: TAB)Modes  +/-)FOV  0)Reset  G)Axes  H)HUD  ESC)Exit");
  // Camera Controls
  yTop -= 15;
  glWindowPos2i(5, yTop);
  if (mode == 2)
    Print("  Camera: L-Click)Look  R-Click)Aim+Shoot  W/S)Move  A/D)Strafe");
  else
    Print("  Camera: Arrows)Look around");
  // Lighting Controls
  yTop -= 15;
  glWindowPos2i(5, yTop);
  if (light) {
    Print("  Lighting: L)Toggle  1/2)Height  3/4)Distance");
  } else {
    Print("  Lighting: L)Toggle");
  }
  // Sky/Time Controls
  yTop -= 15;
  glWindowPos2i(5, yTop);
  if (moveCycle) {
    Print("  Sky/Time: 5)Pause  6/7)Speed (%.2fx)", cycleRate / 0.05);
  } else {
    Print("  Sky/Time: 5)Resume  9)Step  6/7)Speed (%.2fx)", cycleRate / 0.05);
  }
  // Other Controls (combined)
  yTop -= 15;
  glWindowPos2i(5, yTop);
  Print("  Other: O)Tex Optimize %s  F)Fog  P)Pause bullseye  N)Normals",
        textureOptimizations ? "On" : "Off");

  // Mode 2 only: Show status info (at bottom of screen)
  if (showHUD == 2) {
    int yBottom = 5;
    // Title and Mode
    glWindowPos2i(5, yBottom);
    if (mode == 1)
      Print("Mode: Perspective | Angle=%d,%d | FOV=%d | Zoom=%.1f", (int)th,
            (int)ph, fov, dim);
    else
      Print("Mode: First-Person | Angle=%d,%d | Pos=(%.1f,%.1f,%.1f) | FOV=%d",
            (int)th, (int)ph, px, py, pz, fov);
    // Status line
    yBottom += 15;
    glWindowPos2i(5, yBottom);
    // Calculate if it's day or night for status display
    double dayFactor = (Cos(dayNightCycle * 360.0) + 1.0) / 2.0;
    const char* timeOfDay = (dayFactor > 0.5) ? "Day" : "Night";
    const char* cycleState = moveCycle ? "Running" : "Paused";
    Print("Light: %s | Time: %s (%s, %.2fx) | Light Elev=%.1f Dist=%.1f",
          light ? "On" : "Off", timeOfDay, cycleState, cycleRate / 0.05, ylight, ldist);
    // Debug status line
    yBottom += 15;
    glWindowPos2i(5, yBottom);
    Print("Normals: %s | TexOpt: %s | FPS: %.1f",
          showNormals ? "On" : "Off",
          textureOptimizations ? "On" : "Off", fps);
  }
}

/*
 *  Draw dynamic crosshair
 */
void drawCrosshair() {
  int w = glutGet(GLUT_WINDOW_WIDTH);
  int h = glutGet(GLUT_WINDOW_HEIGHT);
  int cx = w / 2;
  int cy = h / 2;

  // Calculate charge
  double charge = 0.0;
  if (chargeStartTime > 0) {
    double now = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    double duration = now - chargeStartTime;
    if (duration > 1.0)
      duration = 1.0;
    charge = duration;
  }

  // Base size and thickness
  double size = 10.0 * (1.0 + charge);
  double thickness = 1.0 + (charge * 2.0);

  // Draw crosshair
  glLineWidth(thickness);
  glColor3f(1.0, 1.0 - charge, 1.0 - charge); // White to Red

  // Use 2D orthographic projection
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, w, 0, h, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);

  glBegin(GL_LINES);
  // Horizontal
  glVertex2d(cx - size, cy);
  glVertex2d(cx + size, cy);
  // Vertical
  glVertex2d(cx, cy - size);
  glVertex2d(cx, cy + size);
  glEnd();

  // Restore state
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  glLineWidth(1.0);
}

/*
 *  Enable lighting with moving light position
 */
void enableLighting() {
  // Calculate day factor: 1.0 = full day, 0.0 = full night
  double dayFactor = (Cos(dayNightCycle * 360.0) + 1.0) / 2.0;
  int isDay = dayFactor > 0.5 ? 1 : 0;

  // Interpolate light intensities based on time of day
  // Day: brighter ambient and diffuse
  // Night: dimmer ambient and diffuse
  float dayAmbient = 0.3;
  float nightAmbient = 0.05;
  float dayDiffuse = 0.9;
  float nightDiffuse = 0.3;

  float ambientIntensity = nightAmbient + dayFactor * (dayAmbient - nightAmbient);
  float diffuseIntensity = nightDiffuse + dayFactor * (dayDiffuse - nightDiffuse);

  float Ambient[] = {ambientIntensity, ambientIntensity, ambientIntensity, 1.0};
  float Diffuse[] = {diffuseIntensity, diffuseIntensity, diffuseIntensity, 1.0};
  float Specular[] = {0.5, 0.5, 0.5, 1.0};

  // Calculate light position from day/night cycle
  // 4 full rotations per complete cycle (2 during day, 2 during night)
  double zhLight = dayNightCycle * 360.0 * 4.0;
  float Position[] = {(ldist * Cos(zhLight)), ylight, (ldist * Sin(zhLight)),
                      1.0};

  // Draw light position as sun or moon
  drawLightBall(Position[0], Position[1], Position[2], 0.15, isDay);

  // Enable lighting
  glEnable(GL_LIGHTING);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);

  // Some specular for highlights
  float white[] = {1, 1, 1, 1};
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32.0);

  // Light0 parameters
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, Ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, Diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, Specular);
  glLightfv(GL_LIGHT0, GL_POSITION, Position);
}

/*
 *  Enable distance fog to blend object color with the background
 *  Uses linear fog based on distance from the viewer.
 */
void enableFog() {
  glEnable(GL_FOG);

  // Fog color roughly matches sky/horizon color and changes with time of day
  double dayFactor = (Cos(dayNightCycle * 360.0) + 1.0) / 2.0; // 0=night,1=day

  // Day and night horizon-like colors (averaged from sky top/bottom)
  float dayFogTop[3] = {0.4f, 0.6f, 0.9f};
  float dayFogBot[3] = {0.7f, 0.85f, 1.0f};
  float nightFogTop[3] = {0.05f, 0.05f, 0.15f};
  float nightFogBot[3] = {0.1f, 0.1f, 0.25f};

  float dayFog[3] = {
      (dayFogTop[0] + dayFogBot[0]) * 0.5f,
      (dayFogTop[1] + dayFogBot[1]) * 0.5f,
      (dayFogTop[2] + dayFogBot[2]) * 0.5f};
  float nightFog[3] = {
      (nightFogTop[0] + nightFogBot[0]) * 0.5f,
      (nightFogTop[1] + nightFogBot[1]) * 0.5f,
      (nightFogTop[2] + nightFogBot[2]) * 0.5f};

  float fogColor[4];
  for (int i = 0; i < 3; ++i) {
    fogColor[i] = nightFog[i] + dayFactor * (dayFog[i] - nightFog[i]);
  }
  fogColor[3] = 1.0f;

  glFogfv(GL_FOG_COLOR, fogColor);
  glFogi(GL_FOG_MODE, GL_LINEAR);

  // Linear fog parameters: heavier at night, lighter during day
  // Night: closer start/end -> stronger fog
  const float fogStartNight = 20.0f;
  const float fogEndNight   = 120.0f;
  // Day: farther start/end -> very subtle fog
  const float fogStartDay = 95.0f;
  const float fogEndDay   = 350.0f;

  float fogStart = fogStartNight + dayFactor * (fogStartDay - fogStartNight);
  float fogEnd   = fogEndNight   + dayFactor * (fogEndDay   - fogEndNight);

  glFogf(GL_FOG_START, fogStart);
  glFogf(GL_FOG_END, fogEnd);

  glHint(GL_FOG_HINT, GL_NICEST);
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display() {
  //  Erase the window and the depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Configure distance fog (color based on day/night)
  if (fog) enableFog();
  else glDisable(GL_FOG);

  // Draw sky background first (before any transformations)
  drawSky(dayNightCycle);

  //  Undo previous transformations
  glLoadIdentity();
  //  Set camera/view
  setViewMode(mode, th, ph, dim, px, py, pz);
  //  Enable Z-buffering
  glEnable(GL_DEPTH_TEST);
  //  Use smooth shading
  glShadeModel(GL_SMOOTH);

  // Lighting setup
  if (light) enableLighting();
  else glDisable(GL_LIGHTING);

  // ===== OPAQUE PASS: Draw all opaque objects first =====
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);

  // Draw bullseyes (animated)
  // showNormals controls whether normal vectors are drawn for debugging
  drawBullseyeScene(zhTargets, showNormals, woodTexture);

  // Enable back-face culling for terrain to improve performance
  glEnable(GL_CULL_FACE);

  // Draw ground terrain (steepness, size, groundY, texture, showNormals)
  const double groundSize = 45.0;
  const double groundY = -3.0;
  drawGround(0.5, groundSize, groundY, groundTexture, showNormals);

  // Draw vast mountain ring surrounding the ground island (bowl-like)
  // innerR should match ground size for a seamless join
  // Overlap mountain ring slightly with ground to avoid gap
  const double overlap = 5.0; // increase overlap to fully seal
  drawMountainRing(groundSize - overlap, 200.0, groundY, mountainTexture,
                   showNormals, 32.0);

  glDisable(GL_CULL_FACE);

  // Draw tree trunks and branches (opaque, uses bark texture)
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW); // Tree geometry winds clockwise; treat CW as front
  drawTreeScene(zhTrees, showNormals, barkTexture, 0);
  glFrontFace(GL_CCW); // Restore default front-face winding
  glDisable(GL_CULL_FACE);

  // Draw Arrow
  drawArrow(&arrow, showNormals);

  // ===== TRANSPARENT PASS: Draw all transparent objects last =====
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE); // IMPORTANT: disable depth writing for transparency

  // Draw tree leaves (transparent)
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, leafTexture);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.1f);
  drawTreeLeaves(zhTrees, leafTexture);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_TEXTURE_2D);

  // Restore render state
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);

  //  Start white coloring
  glColor3f(1, 1, 1);
  glDisable(GL_LIGHTING); // Disable lighting for HUD and axes
  glDisable(GL_FOG);      // Disable fog for HUD and crosshair overlays

  // Draw "overlay" elements
  if (axes) drawAxes(5.0); // Draw axes
  drawHUD(); // Draw HUD (handles its own display based on mode)
  if (mode == 2) drawCrosshair(); // Draw Crosshair in First-Person mode

  //  Present frame
  ErrCheck("display");
  // glFlush();
  glutSwapBuffers();
}

/*
 *  GLUT calls this routine when an arrow key is pressed (key down)
 */
void specialDown(int key, int x, int y) {
  if (mode != 2) {
    // Other modes: step angles immediately (legacy behavior)
    if (key == GLUT_KEY_RIGHT)
      th += 5.0;
    else if (key == GLUT_KEY_LEFT)
      th -= 5.0;
    else if (key == GLUT_KEY_UP)
      ph += 5.0;
    else if (key == GLUT_KEY_DOWN)
      ph -= 5.0;
    // wrap angles
    if (th >= 360.0)
      th -= 360.0;
    else if (th < 0.0)
      th += 360.0;
    if (ph >= 360.0)
      ph -= 360.0;
    else if (ph < 0.0)
      ph += 360.0;
    //  Reproject and redraw
    Project(mode, fov, asp, dim);
    glutPostRedisplay();
  }
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch, int x, int y) {
  //  Exit on ESC
  if (ch == 27)
    exit(0);
  //  Reset view angle
  else if (ch == '0') {
    px = 0;
    py = 0;
    pz = 30;
    fov = 55;
    dim = 25.0;
    // Reset view angles depending on mode
    if (mode == 2) {
      th = 0;
      ph = 0;
    } else {
      th = -45;
      ph = 45;
    }
  }
  //  Movement keys (first-person only): set pressed flags for smooth motion
  else if (mode == 2 && (ch == 'w' || ch == 'W' || ch == 'a' || ch == 'A' ||
                         ch == 's' || ch == 'S' || ch == 'd' || ch == 'D')) {
    if (ch == 'w' || ch == 'W')
      kW = 1;
    else if (ch == 's' || ch == 'S')
      kS = 1;
    else if (ch == 'a' || ch == 'A')
      kA = 1;
    else if (ch == 'd' || ch == 'D')
      kD = 1;
    return; // no further processing for these keys on key-down
  }
  //  Toggle axes
  else if (ch == 'g' || ch == 'G')
    axes = 1 - axes;
  //  Cycle HUD modes (0=hint, 1=controls, 2=all)
  else if (ch == 'h' || ch == 'H')
    showHUD = (showHUD + 1) % 3;
  //  Toggle lighting
  else if (ch == 'l' || ch == 'L')
    light = 1 - light;
  //  Light elevation (1=raise, 2=lower)
  else if (ch == '1')
    ylight += 0.1;
  else if (ch == '2')
    ylight -= 0.1;
  //  Light distance (3=increase, 4=decrease)
  else if (ch == '3') {
    ldist += 0.5;
    if (ldist > 50.0)
      ldist = 50.0;
  } else if (ch == '4') {
    ldist -= 0.5;
    if (ldist < 0.5)
      ldist = 0.5;
  }

  //  Pause/Resume bullseye motion
  else if (ch == 'p' || ch == 'P')
    moveTargets = 1 - moveTargets;
  //  Toggle normals debug
  else if (ch == 'n' || ch == 'N')
    showNormals = 1 - showNormals;
  //  Pause/resume day/night cycle
  else if (ch == '5')
    moveCycle = 1 - moveCycle;
  //  Increase cycle speed
  else if (ch == '6') {
    cycleRate += 0.01;
    if (cycleRate > 0.5)
      cycleRate = 0.5;
  }
  //  Decrease cycle speed
  else if (ch == '7') {
    cycleRate -= 0.01;
    if (cycleRate < 0.01)
      cycleRate = 0.01;
  }
  //  Manually step time when paused
  else if (ch == '9' && !moveCycle)
    dayNightCycle = fmod(dayNightCycle + 0.01, 1.0);
  //  Toggle fog
  else if (ch == 'f' || ch == 'F')
    fog = 1 - fog;
  //  Toggle projection mode (TAB key)
  else if (ch == 9) {
    mode = (mode == 1) ? 2 : 1; // Toggle between 1 and 2

    // When entering first-person, level the pitch and face toward -Z from +Z
    // position
    if (mode == 2) {
      th = 0.0; // th=0 faces -Z in this convention
      ph = 0.0; // look straight ahead
    } else {
      // Reset to default orbit view angles when leaving FP mode
      th = -45.0;
      ph = 45.0;
      // Ensure any mouse-look drag is cleared when leaving FP
      mouseLook = 0;
      leftMouseDown = 0;
      rightMouseDown = 0;
    }
  }
  //  Change field of view angle
  else if (ch == '-' && fov > 1) {
    fov--;
  } else if (ch == '+' && fov < 179) {
    fov++;
  }
  //  Zoom in/out for orbit modes (adjust dim)
  else if (ch == '[' && dim < 100.0) {
    if (mode == 1)
      dim += 2.0;
  } else if (ch == ']' && dim > 5.0) {
    if (mode == 1)
      dim -= 2.0;
  }
  //  Toggle texture filtering optimizations (mipmapping + anisotropy)
  else if (ch == 'o' || ch == 'O') {
    textureOptimizations = 1 - textureOptimizations;
    applyTextureFiltering(groundTexture);
    applyTextureFiltering(mountainTexture);
    applyTextureFiltering(woodTexture);
    applyTextureFiltering(barkTexture);
    applyTextureFiltering(leafTexture);
  }
  //  Update projection
  Project(mode, fov, asp, dim);
  //  Tell GLUT it is necessary to redisplay the scene
  glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a normal key is released (to clear movement
 * flags)
 */
void keyUp(unsigned char ch, int x, int y) {
  if (mode == 2) {
    if (ch == 'w' || ch == 'W')
      kW = 0;
    else if (ch == 's' || ch == 'S')
      kS = 0;
    else if (ch == 'a' || ch == 'A')
      kA = 0;
    else if (ch == 'd' || ch == 'D')
      kD = 0;
  }
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width, int height) {
  //  Ratio of the width to the height of the window
  asp = (height > 0) ? (double)width / height : 1;
  //  Set the viewport to the entire window
  glViewport(0, 0, width, height);
  //  Set projection
  Project(mode, fov, asp, dim);
}

/*
 *  GLUT calls this routine when there is nothing else to do
 *  Animate side-to-side motion by advancing zh
 */
void idle() {
  static double lastT = 0.0;
  double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
  if (lastT == 0.0) {
    lastT = t;
    lastFPSTime = t;
  }
  double dt = t - lastT;
  lastT = t;

  // Calculate FPS every 0.5 seconds
  frameCount++;
  if (t - lastFPSTime >= 0.5) {
    fps = frameCount / (t - lastFPSTime);
    frameCount = 0;
    lastFPSTime = t;
  }

  // First-person: update movement from WASD continuously
  if (mode == 2) {
    fpUpdateMove(th, kW, kS, kA, kD, moveStep, dt, &px, &pz);
  }

  // Light position is calculated from dayNightCycle in enableLighting()
  // Bullseyes move only when enabled
  if (moveTargets)
    zhTargets = fmod(zhTargets + targetRate * dt, 360.0);
  // Trees sway continuously (gentle)
  zhTrees = fmod(zhTrees + 25.0 * dt, 360.0);
  // Day/Night cycle advances when enabled
  if (moveCycle)
    dayNightCycle = fmod(dayNightCycle + cycleRate * dt, 1.0);

  // Update arrow physics
  updateArrow(&arrow, dt);

  glutPostRedisplay();
}

/*
 *  Mouse handlers for first-person look (click-drag to look)
 */
void mouse(int button, int state, int x, int y) {
  if (mode != 2)
    return;

  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
      leftMouseDown = 1;
      mouseLook = 1;
      lastX = x;
      lastY = y;
    } else if (state == GLUT_UP) {
      leftMouseDown = 0;
      if (!rightMouseDown)
        mouseLook = 0;
    }
    glutPostRedisplay();
  } else if (button == GLUT_RIGHT_BUTTON) {
    // Right click to shoot (Charge mechanic) and allow look-drag while aiming
    if (state == GLUT_DOWN) {
      rightMouseDown = 1;
      mouseLook = 1;
      lastX = x;
      lastY = y;
      // Start charging
      chargeStartTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    } else if (state == GLUT_UP) {
      rightMouseDown = 0;
      if (!leftMouseDown)
        mouseLook = 0;

      if (chargeStartTime > 0.0) {
        // Release to shoot
        double now = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
        double duration = now - chargeStartTime;

        // Map duration to speed
        // Min speed 10, Max speed 50
        // Max charge time 1.0 second
        double maxChargeTime = 1.0;
        double minSpeed = 10.0;
        double maxSpeed = 50.0;

        if (duration > maxChargeTime)
          duration = maxChargeTime;

        double speed =
            minSpeed + (duration / maxChargeTime) * (maxSpeed - minSpeed);

        shootArrow(&arrow, px, py, pz, th, ph, speed);
      }

      // Reset charge time
      chargeStartTime = 0;
    }
  }
}

void motion(int x, int y) {
  if (mode != 2 || !mouseLook)
    return;
  int dx = x - lastX;
  int dy = y - lastY;
  lastX = x;
  lastY = y;
  // Update yaw/pitch from mouse delta
  th += dx * mouseSensitivity;
  if (th >= 360.0)
    th -= 360.0;
  else if (th < 0.0)
    th += 360.0;
  ph -= dy * mouseSensitivity;
  if (ph > 89.0)
    ph = 89.0;
  else if (ph < -89.0)
    ph = -89.0;
  glutPostRedisplay();
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc, char *argv[]) {
  //  Initialize GLUT and process user parameters
  glutInit(&argc, argv);
  //  Request double buffered, true color window with Z buffering
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  //  Request 1000 x 700 pixel window
  glutInitWindowSize(1000, 700);
  //  Create the window
  glutCreateWindow("Final Project: Richard Roberson");
#ifdef USEGLEW
  //  Initialize GLEW
  if (glewInit() != GLEW_OK)
    Fatal("Error initializing GLEW\n");
#endif
  //  Detect anisotropic filtering support once a GL context exists
  detectAnisoSupport();
  //  Load ground texture
  groundTexture = LoadTexBMP("textures/ground.bmp");
  //  Load mountain texture (surrounding ring)
  mountainTexture = LoadTexBMP("textures/ground2.bmp");
  //  Load wood texture for bullseyes
  woodTexture = LoadTexBMP("textures/wood.bmp");
  //  Load bark texture for trees
  barkTexture = LoadTexBMP("textures/bark.bmp");
  //  Load leaf texture with proper alpha settings
  leafTexture = LoadTexBMP("textures/leaf.bmp");
  //  Apply preferred filtering mode to all textures
  applyTextureFiltering(groundTexture);
  applyTextureFiltering(mountainTexture);
  applyTextureFiltering(woodTexture);
  applyTextureFiltering(barkTexture);
  applyTextureFiltering(leafTexture);
  //  Tell GLUT to call "display" when the scene should be drawn
  glutDisplayFunc(display);
  //  Tell GLUT to call "idle" when there is nothing else to do (animate)
  glutIdleFunc(idle);
  //  Tell GLUT to call "reshape" when the window is resized
  glutReshapeFunc(reshape);
  //  Tell GLUT to call arrow key handler (down); arrows are used in non-FP
  //  modes only
  glutSpecialFunc(specialDown);
  // No specialUp needed
  //  Tell GLUT to call "key" when a key is pressed
  glutKeyboardFunc(key);
  //  Tell GLUT to call keyUp when a key is released (for WASD)
  glutKeyboardUpFunc(keyUp);
  //  Mouse look in first-person
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  //  Pass control to GLUT so it can interact with the user
  glutMainLoop();
  return 0;
}
