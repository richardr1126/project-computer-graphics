/*
 *  Final Project: Richard Roberson
 *
 *  Key Bindings:
 *  View Controls:
 *    TAB    Cycle view modes: Orthographic -> Perspective (orbit) -> First-Person
 *    +/-    Change field of view (perspective modes)
 *    0      Reset view (camera position/angles, FOV)
 *    g/G    Toggle axes display
 *    h/H    Toggle HUD
 *    ESC    Exit
 *  Camera Controls:
 *    Mouse drag Look around (first-person mode)
 *    arrows     Look around (orthographic & perspective orbit modes)
 *    w/s        Move forward/backward (first-person mode only)
 *    a/d        Strafe left/right (first-person mode only)
 *  Lighting Controls:
 *    l/L    Toggle lighting on/off
 *    1/2    Raise/lower light height
 *    3/4    Increase/decrease light distance
 *    5      Pause/resume light rotation
 *    6      Manual light rotation (when paused)
 *    f/F    Toggle smooth/flat shading
 *  Object Controls:
 *    p/P    Pause/resume bullseye motion
 *    n/N    Toggle normals debug lines
 */
//  Include custom modules
#include "utils.h"
#include "objects.h"
#include "view.h"

//  Global state variables
// View parameters
double th = 0.0; //  Azimuth of view angle (degrees)
double ph = 0.0;  //  Elevation of view angle (degrees)
int axes = 0; //  Display axes
int mode = 2; //  View mode: 0=Orthogonal, 1=Perspective (orbit), 2=First-Person
int fov = 55; //  Field of view for perspective
double asp = 1;   //  Aspect ratio
double dim = 8.0; //  World size for projection
int showHUD = 1;         //  HUD visibility toggle
//  First-person camera
double px = 0, py = 0, pz = 17; // Position of the camera in world coords
double moveStep = 7.0;          // Movement speed (units per second)
int kW = 0, kA = 0, kS = 0, kD = 0;           // WASD movement keys
// Mouse look state (first-person)
int mouseLook = 0;     // 1 while left button held for looking
int lastX = 0, lastY = 0; // last mouse position
double mouseSensitivity = 0.15; // degrees per pixel (lower for smoother feel)
// Bullseye motion
double zhTargets = 0;     // Animation angle for bullseye motion (degrees)
int moveTargets  = 1;     // Toggle bullseye motion
double targetRate = 90.0; // default target motion speed (degrees per second)
//  Lighting
int light = 1;           // Lighting toggle
double lightRate = 70.0; // light rotation speed (degrees per second)
int moveLight = 1;       // Toggle light motion
double zhLight = 0;      // Animation angle for light motion (degrees)
double ylight = 0.0;     // Elevation of the light
double ldist = 8.0;      // Light distance from origin in XZ plane
int smooth = 1;          //  Shading mode: 1=Smooth, 0=Flat
//  Debug helpers
int showNormals = 0;       //  Toggle drawing of normal vectors
//  Textures
unsigned int groundTexture = 0; // Ground texture ID
unsigned int woodTexture = 0;   // Wood texture ID for bullseyes

/*
 *  Draw HUD with controls and status information
 */
void drawHUD()
{
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
    int yBottom = 5;
    
    glColor3f(1, 1, 1);
    
    // Title and Mode
    glWindowPos2i(5, yBottom);
    if (mode == 0)
        Print("Mode: Orthogonal | Angle=%d,%d", (int)th, (int)ph);
    else if (mode == 1)
        Print("Mode: Perspective | Angle=%d,%d | FOV=%d", (int)th, (int)ph, fov);
    else
        Print("Mode: First-Person | Angle=%d,%d | Pos=(%.1f,%.1f,%.1f) | FOV=%d", (int)th, (int)ph, px, py, pz, fov);
    // Status line
    yBottom += 15;
    glWindowPos2i(5, yBottom);
    Print("Light: %s | Elev=%.1f | Dist=%.1f | Axes=%s | Shade=%s",
          light?"On":"Off", ylight, ldist,
          axes?"On":"Off", smooth?"Smooth":"Flat");
    // Debug status line
    yBottom += 15;
    glWindowPos2i(5, yBottom);
    Print("Normals: %s", showNormals?"On":"Off");
    
    
    // Controls section
    int yTop = windowHeight - 15;
    // Controls section header
    glWindowPos2i(5, yTop);
    Print("Controls:");
    // View Controls
    yTop -= 15;
    glWindowPos2i(5, yTop);
    if (mode == 0)
        Print("  View: TAB)Modes  0)Reset  G)Axes  H)HUD  ESC)Exit");
    else
        Print("  View: TAB)Modes  +/-)FOV  0)Reset  G)Axes  H)HUD  ESC)Exit");
    // Camera Controls
    yTop -= 15;
    glWindowPos2i(5, yTop);
    if (mode == 2)
        Print("  Camera: Left-click drag)Look around  W/S)Forward/Back  A/D)Strafe Left/Right");
    else
        Print("  Camera: Arrows)Look around");
    // Lighting Controls
    yTop -= 15;
    glWindowPos2i(5, yTop);
    if (light) {
        //Print("  Lighting: L)Toggle  1/2)Height  3/4)Distance  5)Pause  6)Manual  F)Smooth/Flat");
        if (moveLight) {
            Print("  Lighting: L)Toggle  1/2)Height  3/4)Distance  5)Pause  F)Smooth/Flat");
        } else {
            Print("  Lighting: L)Toggle  1/2)Height  3/4)Distance  5)Resume  6)Step  F)Smooth/Flat");
        }
    } else {
        Print("  Lighting: L)Toggle");
    }
    // Object Controls
    yTop -= 15;
    glWindowPos2i(5, yTop);
    Print("  Object: P)Pause/Resume bullseye  N)Debug normals");
}

/*
 *  Enable lighting with moving light position
 */
void enableLighting()
{
    // Translate intensities to color vectors
    float Ambient[]  = {0.2, 0.2, 0.2, 1.0};
    float Diffuse[]  = {0.8, 0.8, 0.8, 1.0};
    float Specular[] = {0.5, 0.5, 0.5, 1.0};

    // Light position moving around Y axis with zh
    float Position[] = {(ldist * Cos(zhLight)), ylight, (ldist * Sin(zhLight)), 1.0};

    // Draw light position as small white ball (unlit)
    drawLightBall(Position[0], Position[1], Position[2], 0.15);

    // Enable lighting
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    // Some specular for highlights
    float white[] = {1,1,1,1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32.0);

    // Light0 parameters
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT , Ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE , Diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, Specular);
    glLightfv(GL_LIGHT0, GL_POSITION, Position);
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
    //  Erase the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //  Undo previous transformations
    glLoadIdentity();
    //  Set camera/view
    setViewMode(mode, th, ph, dim, px, py, pz);

    //  Enable Z-buffering
    glEnable(GL_DEPTH_TEST);
    //  Flat or smooth shading
    glShadeModel(smooth ? GL_SMOOTH : GL_FLAT);
    //  Enable back face culling
    // glEnable(GL_CULL_FACE);

    // Lighting setup
    if (light) enableLighting();
    else glDisable(GL_LIGHTING);

    // Draw the scene (bullseyes use their own animation angle)
    // showNormals controls whether normal vectors are drawn for debugging
    drawBullseyeScene(zhTargets, showNormals, woodTexture);

    // Draw ground terrain (steepness, size, groundY, texture, showNormals)
    drawGround(0.5, 45.0, -3.0, groundTexture, showNormals);
    
    //  Start white coloring
    glColor3f(1, 1, 1);
    glDisable(GL_LIGHTING); // Disable lighting for HUD and axes
    if (axes) drawAxes(5.0); // Draw axes
    if (showHUD) drawHUD(); // Draw HUD

    //  Make sure changes appear onscreen
    ErrCheck("display");
    glFlush();
    glutSwapBuffers();
}

/*
 *  GLUT calls this routine when an arrow key is pressed (key down)
 */
void specialDown(int key, int x, int y)
{
    if (mode != 2)
    {
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
        if (th >= 360.0) th -= 360.0; else if (th < 0.0) th += 360.0;
        if (ph >= 360.0) ph -= 360.0; else if (ph < 0.0) ph += 360.0;
        //  Reproject and redraw
        Project(mode, fov, asp, dim);
        glutPostRedisplay();
    }
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch, int x, int y)
{
    //  Exit on ESC
    if (ch == 27)
        exit(0);
    //  Reset view angle
    else if (ch == '0')
    {
        px = 0;
        py = 0;
        pz = 17;
        fov = 55;
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
    else if (mode == 2 && (ch == 'w' || ch == 'W' || ch == 'a' || ch == 'A' || ch == 's' || ch == 'S' || ch == 'd' || ch == 'D'))
    {
        if (ch == 'w' || ch == 'W') kW = 1;
        else if (ch == 's' || ch == 'S') kS = 1;
        else if (ch == 'a' || ch == 'A') kA = 1;
        else if (ch == 'd' || ch == 'D') kD = 1;
        return; // no further processing for these keys on key-down
    }
    //  Toggle axes
    else if (ch == 'g' || ch == 'G')
        axes = 1 - axes;
    //  Toggle HUD
    else if (ch == 'h' || ch == 'H')
        showHUD = 1 - showHUD;
    //  Toggle lighting
    else if (ch == 'l' || ch == 'L')
        light = 1 - light;
    //  Light elevation (1=raise, 2=lower)
    else if (ch == '1')
        ylight += 0.1;
    else if (ch == '2')
        ylight -= 0.1;
    //  Light distance (3=increase, 4=decrease)
    else if (ch == '3')
    {
        ldist += 0.5;
        if (ldist > 50.0) ldist = 50.0;
    }
    else if (ch == '4')
    {
        ldist -= 0.5;
        if (ldist < 0.5) ldist = 0.5;
    }
    //  Toggle light rotation
    else if (ch == '5')
        moveLight = 1 - moveLight;
    //  Manually step light rotation when paused
    else if (ch == '6' && !moveLight)
        zhLight = fmod(zhLight + 5.0, 360.0);
    //  Toggle smooth/flat shading
    else if (ch == 'f' || ch == 'F')
        smooth = 1 - smooth;
    //  Pause/Resume bullseye motion
    else if (ch == 'p' || ch == 'P')
        moveTargets = 1 - moveTargets;
    //  Toggle normals debug
    else if (ch == 'n' || ch == 'N')
        showNormals = 1 - showNormals;
    //  Toggle projection mode (TAB key)
    else if (ch == 9)
    {
        mode = (mode + 1) % 3; // cycle 0,1,2,0,1,...
        
        // When entering first-person, level the pitch and face toward -Z from +Z position
        if (mode == 2)
        {
            th = 0.0; // th=0 faces -Z in this convention
            ph = 0.0; // look straight ahead
        } else {
            // Reset to default orbit view angles when leaving FP mode
            th = -45.0;
            ph = 45.0;
            // Ensure any mouse-look drag is cleared when leaving FP
            mouseLook = 0;
        }
    }
    //  Change field of view angle
    else if (ch == '-' && fov > 1) {
        if (mode > 0) fov--;
    } else if (ch == '+' && fov < 179) {
        if (mode > 0) fov++;
    }
    //  Update projection
    Project(mode, fov, asp, dim);
    //  Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a normal key is released (to clear movement flags)
 */
void keyUp(unsigned char ch, int x, int y)
{
    if (mode == 2)
    {
        if (ch == 'w' || ch == 'W') kW = 0;
        else if (ch == 's' || ch == 'S') kS = 0;
        else if (ch == 'a' || ch == 'A') kA = 0;
        else if (ch == 'd' || ch == 'D') kD = 0;
    }
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width, int height)
{
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
void idle()
{
    static double lastT = 0.0;
    double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    if (lastT == 0.0) lastT = t;
    double dt = t - lastT;
    lastT = t;

    // First-person: update movement from WASD continuously
    if (mode == 2)
    {
        fpUpdateMove(th, kW, kS, kA, kD, moveStep, dt, &px, &pz);
    }

    // Light moves only when enabled
    if (moveLight)
        zhLight = fmod(zhLight + lightRate * dt, 360.0);
    // Bullseyes move only when enabled
    if (moveTargets)
        zhTargets = fmod(zhTargets + targetRate * dt, 360.0);
    glutPostRedisplay();
}

/*
 *  Mouse handlers for first-person look (click-drag to look)
 */
void mouse(int button, int state, int x, int y)
{
    if (mode != 2) return;
    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseLook = 1;
            lastX = x;
            lastY = y;
        }
        else if (state == GLUT_UP)
        {
            mouseLook = 0;
        }
        glutPostRedisplay();
    }
}

void motion(int x, int y)
{
    if (mode != 2 || !mouseLook) return;
    int dx = x - lastX;
    int dy = y - lastY;
    lastX = x;
    lastY = y;
    // Update yaw/pitch from mouse delta
    th += dx * mouseSensitivity;
    if (th >= 360.0) th -= 360.0; else if (th < 0.0) th += 360.0;
    ph -= dy * mouseSensitivity;
    if (ph > 89.0) ph = 89.0; else if (ph < -89.0) ph = -89.0;
    glutPostRedisplay();
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc, char *argv[])
{
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
    //  Load ground texture
    groundTexture = LoadTexBMP("textures/ground.bmp");
    //  Load wood texture for bullseyes
    woodTexture = LoadTexBMP("textures/wood.bmp");
    //  Tell GLUT to call "display" when the scene should be drawn
    glutDisplayFunc(display);
    //  Tell GLUT to call "idle" when there is nothing else to do (animate)
    glutIdleFunc(idle);
    //  Tell GLUT to call "reshape" when the window is resized
    glutReshapeFunc(reshape);
    //  Tell GLUT to call arrow key handler (down); arrows are used in non-FP modes only
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
