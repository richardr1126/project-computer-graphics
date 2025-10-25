/*
 *  Objects module - implementation file
 *  Contains drawing functions for 3D objects
 */
#include "utils.h"
#include "objects.h"

/*
 *  Vertex on a unit sphere given angles (degrees)
 */
static void SphereVertex(double th, double ph)
{
    double x = Sin(th) * Cos(ph);
    double y = Cos(th) * Cos(ph);
    double z = Sin(ph);
    glNormal3d(x, y, z);
    // Texture coordinates: spherical mapping
    glTexCoord2d(th / 360.0, (ph + 90.0) / 180.0);
    glVertex3d(x, y, z);
}

/*
 *  Apply translation and orientation for a bullseye given Bullseye struct
 */
static void applyBullseyeTransform(const Bullseye* b)
{
    if (!b) return;
    glTranslatef((float)b->x, (float)b->y, (float)b->z);
    // Build orthonormal basis from direction (dx,dy,dz) and up (ux,uy,uz)
    double dx = b->dx, dy = b->dy, dz = b->dz;
    double ux = b->ux, uy = b->uy, uz = b->uz;
    double D0 = sqrt(dx * dx + dy * dy + dz * dz);
    if (!D0) D0 = 1;
    double X0 = dx / D0, Y0 = dy / D0, Z0 = dz / D0; // local X axis
    double D1 = sqrt(ux * ux + uy * uy + uz * uz);
    if (!D1) D1 = 1;
    double X1 = ux / D1, Y1 = uy / D1, Z1 = uz / D1; // local Y axis
    // Cross to get local Z axis
    double X2 = Y0 * Z1 - Y1 * Z0;
    double Y2 = Z0 * X1 - Z1 * X0;
    double Z2 = X0 * Y1 - X1 * Y0;
    //  Rotation matrix (column-major for OpenGL)
    double mat[16];
    mat[0] = X0;   mat[4] = X1;   mat[ 8] = X2;   mat[12] = 0;
    mat[1] = Y0;   mat[5] = Y1;   mat[ 9] = Y2;   mat[13] = 0;
    mat[2] = Z0;   mat[6] = Z1;   mat[10] = Z2;   mat[14] = 0;
    mat[3] =  0;   mat[7] =  0;   mat[11] =  0;   mat[15] = 1;

    glMultMatrixd(mat);
}

/*
 *  Draw a bullseye from a Bullseye struct
 */
static void drawBullseye(const Bullseye* b, unsigned int texture)
{
    if (!b) return;
    // Save transformation
    glPushMatrix();
    // Translate to (x,y,z), orient by (dx,dy,dz) and (ux,uy,uz)
    applyBullseyeTransform(b);

    // Enable texturing if texture provided
    if (texture)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }

    // Draw concentric rings (bullseye) as a short 3D stack
    int nRings = (b->rings > 0) ? b->rings : 1; // Number of colored rings (validated)
    const double R = (b->radius > 0.0) ? b->radius : 1.0; // Outer radius in world units
    const double step = R / nRings;
    const int d = 10;      // Angular step in degrees
    const double hz = 0.1; // Half-thickness in world units (constant)

    // Draw outer-to-inner rings, alternating colors
    for (int i = 0; i < nRings; ++i)
    {
        double ro = R - i * step; // outer radius of ring
        double ri = ro - step;    // inner radius of ring

        // Alternate colors: specified color/white per ring (colors blend with texture via GL_MODULATE)
        if (i % 2 == 0)
            glColor3f(b->r, b->g, b->b);
        else
            glColor3f(1.0, 1.0, 1.0);

        if (ri > 0.0)
        {
            // Top face (z=+hz) - normals +Z
            glBegin(GL_TRIANGLE_STRIP);
            glNormal3f(0, 0, +1);
            for (int ang = 0; ang <= 360; ang += d)
            {
                double c = Cos(ang);
                double s = Sin(ang);
                if (texture) glTexCoord2d(0.5 + 0.5 * ro * c / R, 0.5 + 0.5 * ro * s / R);
                glVertex3d(ro * c, ro * s, +hz);
                if (texture) glTexCoord2d(0.5 + 0.5 * ri * c / R, 0.5 + 0.5 * ri * s / R);
                glVertex3d(ri * c, ri * s, +hz);
            }
            glEnd();
            // Bottom face (z=-hz) - normals -Z
            glBegin(GL_TRIANGLE_STRIP);
            glNormal3f(0, 0, -1);
            for (int ang = 0; ang <= 360; ang += d)
            {
                double c = Cos(ang);
                double s = Sin(ang);
                if (texture) glTexCoord2d(0.5 + 0.5 * ro * c / R, 0.5 + 0.5 * ro * s / R);
                glVertex3d(ro * c, ro * s, -hz);
                if (texture) glTexCoord2d(0.5 + 0.5 * ri * c / R, 0.5 + 0.5 * ri * s / R);
                glVertex3d(ri * c, ri * s, -hz);
            }
            glEnd();
            // Outer side wall (cylindrical surface at radius ro) - radial outward normals
            glBegin(GL_QUAD_STRIP);
            for (int ang = 0; ang <= 360; ang += d)
            {
                double c = Cos(ang);
                double s = Sin(ang);
                glNormal3d(c, s, 0);
                if (texture) glTexCoord2d(ang / 360.0, 1.0);
                glVertex3d(ro * c, ro * s, +hz);
                glNormal3d(c, s, 0);
                if (texture) glTexCoord2d(ang / 360.0, 0.0);
                glVertex3d(ro * c, ro * s, -hz);
            }
            glEnd();
            // Inner side wall (cylindrical surface at radius ri) - radial inward normals
            glBegin(GL_QUAD_STRIP);
            for (int ang = 0; ang <= 360; ang += d)
            {
                double c = Cos(ang);
                double s = Sin(ang);
                glNormal3d(-c, -s, 0);
                if (texture) glTexCoord2d(ang / 360.0, 0.0);
                glVertex3d(ri * c, ri * s, -hz);
                glNormal3d(-c, -s, 0);
                if (texture) glTexCoord2d(ang / 360.0, 1.0);
                glVertex3d(ri * c, ri * s, +hz);
            }
            glEnd();
        }
        else
        {
            // Center disk (no inner hole)
            // Top
            glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0, 0, +1);
            if (texture) glTexCoord2d(0.5, 0.5);
            glVertex3d(0.0, 0.0, +hz);
            for (int ang = 0; ang <= 360; ang += d)
            {
                double c = Cos(ang);
                double s = Sin(ang);
                if (texture) glTexCoord2d(0.5 + 0.5 * ro * c / R, 0.5 + 0.5 * ro * s / R);
                glVertex3d(ro * c, ro * s, +hz);
            }
            glEnd();
            // Bottom
            glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0, 0, -1);
            if (texture) glTexCoord2d(0.5, 0.5);
            glVertex3d(0.0, 0.0, -hz);
            for (int ang = 0; ang <= 360; ang += d)
            {
                double c = Cos(ang);
                double s = Sin(ang);
                if (texture) glTexCoord2d(0.5 + 0.5 * ro * c / R, 0.5 + 0.5 * ro * s / R);
                glVertex3d(ro * c, ro * s, -hz);
            }
            glEnd();
            // Side cylinder at radius ro
            glBegin(GL_QUAD_STRIP);
            for (int ang = 0; ang <= 360; ang += d)
            {
                double c = Cos(ang);
                double s = Sin(ang);
                glNormal3d(c, s, 0);
                if (texture) glTexCoord2d(ang / 360.0, 1.0);
                glVertex3d(ro * c, ro * s, +hz);
                glNormal3d(c, s, 0);
                if (texture) glTexCoord2d(ang / 360.0, 0.0);
                glVertex3d(ro * c, ro * s, -hz);
            }
            glEnd();
        }
    }

    // Disable texturing after drawing
    if (texture)
    {
        glDisable(GL_TEXTURE_2D);
    }

    // Restore transformation
    glPopMatrix();
}

/*
 *  Draw the scene with multiple bullseye targets
 */
static void drawBullseyeNormals(const Bullseye* b)
{
    if (!b) return;

    // Save transformation
    glPushMatrix();
    // Translate to (x,y,z), orient by (dx,dy,dz) and (ux,uy,uz)
    applyBullseyeTransform(b);

    // Below was generated with AI assistance
    // Draws normals for a bullseye (unlit lines)
    const double step = b->radius / b->rings;
    const double hz = 0.1;
    const int d = 30; // sparser for visibility
    const double L = 0.3; // line length
    glColor3f(1,1,0); // Apply yellow color for normals
    glBegin(GL_LINES);
    // Top and bottom face normals (sample at a few angles on outermost ring)
    for (int ang = 0; ang < 360; ang += d)
    {
        double c = Cos(ang), s = Sin(ang);
        // top
        glVertex3d(b->radius * c, b->radius * s, +hz);
        glVertex3d(b->radius * c, b->radius * s, +hz + L);
        // bottom
        glVertex3d(b->radius * c, b->radius * s, -hz);
        glVertex3d(b->radius * c, b->radius * s, -hz - L);
    }
    // Side walls (outer and inner of one ring)
    for (int ang = 0; ang < 360; ang += d)
    {
        double c = Cos(ang), s = Sin(ang);
        // outer wall normal
        glVertex3d(b->radius * c, b->radius * s, 0);
        glVertex3d(b->radius * c + L * c, b->radius * s + L * s, 0);
        // inner wall normal on innermost radius
        double ri = (b->rings > 0) ? (b->radius - step) : (b->radius * 0.5);
        if (ri > 0)
        {
            glVertex3d(ri * c, ri * s, 0);
            glVertex3d(ri * c - L * c, ri * s - L * s, 0);
        }
    }
    glEnd();
    glPopMatrix();
}

/*
 *  Draw a lit sphere using latitude-longitude quads
 *  at (x,y,z) with radius r and angular increment inc degrees
 */
static void drawBall(double x, double y, double z, double r, int inc)
{
    if (inc < 1) inc = 1;
    // Save transform and move/scale
    glPushMatrix();
    glTranslated(x, y, z);
    glScaled(r, r, r);
    // Bands of latitude
    for (int ph = -90; ph < 90; ph += inc)
    {
        glBegin(GL_QUAD_STRIP);
        for (int th = 0; th <= 360; th += 2 * inc)
        {
            SphereVertex(th, ph);
            SphereVertex(th, ph + inc);
        }
        glEnd();
    }
    glPopMatrix();
}
/*
 *  Helper to draw normals for a bullseye if requested
 */
static void drawBullseyeWithNormals(const Bullseye* b, unsigned int texture, int showNormals)
{
    drawBullseye(b, texture);
    if (showNormals)
    {
        GLboolean wasLit = glIsEnabled(GL_LIGHTING);
        if (wasLit) glDisable(GL_LIGHTING);
        drawBullseyeNormals(b);
        if (wasLit) glEnable(GL_LIGHTING);
    }
}

void drawBullseyeScene(double zh, int showNormals, unsigned int texture)
{
    double off = 3.0 * Sin(zh);

    Bullseye b1 = {
        .x = 0.0, .y = 0.0, .z = 0.0,
        .dx = 1.0, .dy = 0.0, .dz = 0.0,
        .ux = 0.0, .uy = 1.0, .uz = 0.0,
        .radius = 2.0,
        .rings = 6,
        .r = 1.0, .g = 0.0, .b = 0.0
    };
    drawBullseyeWithNormals(&b1, texture, showNormals);

    double cos45 = Cos(45), sin45 = Sin(45);
    double moveX1 = cos45 * off;
    double moveZ1 = -sin45 * off;
    Bullseye b2 = {
        .x = -3.0 + moveX1, .y = 5.0, .z = -1.0 + moveZ1,
        .dx = cos45, .dy = 0.0, .dz = sin45,
        .ux = 0.0, .uy = 1.0, .uz = 0.0,
        .radius = 1.25,
        .rings = 5,
        .r = 0.0, .g = 0.0, .b = 1.0
    };
    drawBullseyeWithNormals(&b2, texture, showNormals);

    Bullseye b3 = {
        .x = 3.0 + off, .y = 5.0, .z = 1.0,
        .dx = Cos(-30), .dy = 0.0, .dz = Sin(-30),
        .ux = 0.0, .uy = 1.0, .uz = 0.0,
        .radius = 1.25,
        .rings = 4,
        .r = 0.0, .g = 1.0, .b = 0.0
    };
    drawBullseyeWithNormals(&b3, texture, showNormals);

    double cos60 = Cos(60), sin60 = Sin(60);
    double moveX3 = cos60 * off;
    double moveZ3 = -sin60 * off;
    Bullseye b4 = {
        .x = -3.0 + moveX3, .y = 1.5, .z = -1.0 + moveZ3,
        .dx = cos60, .dy = 0.0, .dz = sin60,
        .ux = 0.0, .uy = 1.0, .uz = 0.0,
        .radius = 1.25,
        .rings = 4,
        .r = 1.0, .g = 0.0, .b = 1.0
    };
    drawBullseyeWithNormals(&b4, texture, showNormals);

    Bullseye b5 = {
        .x = 3.0, .y = 2.5, .z = 1.0 + off,
        .dx = 0.0, .dy = 0.0, .dz = 1.0,
        .ux = 0.0, .uy = 1.0, .uz = 0.0,
        .radius = 1.25,
        .rings = 5,
        .r = 0.0, .g = 1.0, .b = 1.0
    };
    drawBullseyeWithNormals(&b5, texture, showNormals);
}

/*
 *  Draw a small sphere to represent the light (unlit so it appears emissive)
 */
void drawLightBall(double x, double y, double z, double r)
{
    glPushMatrix();
    // Draw unlit so it appears emissive and not affected by scene lighting
    GLboolean wasLit = glIsEnabled(GL_LIGHTING);
    glDisable(GL_LIGHTING);
    
    // Draw as bright white/yellow color to represent light
    glColor3f(1, 1, 0.8);
    drawBall(x, y, z, r, 3);
    
    if (wasLit) glEnable(GL_LIGHTING);
    glPopMatrix();
}

/*
 *  Compute height for terrain at given (x,z) position
 *  Uses combination of sine waves to create varied terrain
 *  steepness: multiplier for terrain height variation (1.0 = default)
 */
static double terrainHeight(double x, double z, double steepness)
{
    // Combine multiple sine waves for interesting terrain
    double h = 0.0;
    h += 0.3 * sin(x * 0.5) * cos(z * 0.5);
    h += 0.2 * sin(x * 0.8 + z * 0.3);
    h += 0.15 * cos(x * 1.2 - z * 0.7);
    return h * steepness;
}

/*
 *  Compute normal vector for terrain at position (x,z)
 *  Uses finite differences to approximate the normal
 */
static void terrainNormal(double x, double z, double steepness, double* nx, double* ny, double* nz)
{
    const double delta = 0.1;
    // Sample heights around the point
    double hL = terrainHeight(x - delta, z, steepness);
    double hR = terrainHeight(x + delta, z, steepness);
    double hD = terrainHeight(x, z - delta, steepness);
    double hU = terrainHeight(x, z + delta, steepness);
    
    // Compute tangent vectors
    double tx = 2.0 * delta;
    double ty = hR - hL;
    double tz = 0.0;
    
    double ux = 0.0;
    double uy = hU - hD;
    double uz = 2.0 * delta;
    
    // Cross product to get normal (flipped order for correct direction)
    double cx = uy * tz - uz * ty;
    double cy = uz * tx - ux * tz;
    double cz = ux * ty - uy * tx;
    
    // Normalize
    double len = sqrt(cx * cx + cy * cy + cz * cz);
    if (len > 0.0)
    {
        *nx = cx / len;
        *ny = cy / len;
        *nz = cz / len;
    }
    else
    {
        *nx = 0.0;
        *ny = 1.0;
        *nz = 0.0;
    }
}

/*
 *  Draw ground terrain with varied height
 *  steepness: terrain height multiplier
 *  size: ground extends from -size to +size in X and Z
 *  groundY: base height offset in Y direction
 *  texture: OpenGL texture ID (0 for no texture)
 *  showNormals: whether to draw normal vectors
 */
void drawGround(double steepness, double size, double groundY, unsigned int texture, int showNormals)
{
    const double step = 0.5;  // Grid resolution
    
    // Set material properties for ground - minimal specular to avoid stretching artifacts
    float groundSpecular[] = {0.05, 0.05, 0.05, 1.0}; // Very low specular
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, groundSpecular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 2.0); // Low shininess for matte surface
    
    // Enable texturing if texture provided
    if (texture)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glColor3f(1.0, 1.0, 1.0); // White to show true texture colors
    }
    else
    {
        glColor3f(0.3, 0.5, 0.2); // Green-ish ground color
    }
    
    // Texture coordinate scale (how many times texture repeats across ground)
    const double texScale = 0.1;
    
    // Draw individual quads for proper normal calculation per vertex
    glBegin(GL_QUADS);
    for (double x = -size; x < size; x += step)
    {
        for (double z = -size; z < size; z += step)
        {
            // Four corners of the quad with individual normals
            double h1, h2, h3, h4;
            double nx1, ny1, nz1;
            double nx2, ny2, nz2;
            double nx3, ny3, nz3;
            double nx4, ny4, nz4;
            
            // Bottom-left corner (x, z)
            h1 = terrainHeight(x, z, steepness);
            terrainNormal(x, z, steepness, &nx1, &ny1, &nz1);
            glNormal3d(nx1, ny1, nz1);
            if (texture) glTexCoord2d(x * texScale, z * texScale);
            glVertex3d(x, groundY + h1, z);
            
            // Bottom-right corner (x+step, z)
            h2 = terrainHeight(x + step, z, steepness);
            terrainNormal(x + step, z, steepness, &nx2, &ny2, &nz2);
            glNormal3d(nx2, ny2, nz2);
            if (texture) glTexCoord2d((x + step) * texScale, z * texScale);
            glVertex3d(x + step, groundY + h2, z);
            
            // Top-right corner (x+step, z+step)
            h3 = terrainHeight(x + step, z + step, steepness);
            terrainNormal(x + step, z + step, steepness, &nx3, &ny3, &nz3);
            glNormal3d(nx3, ny3, nz3);
            if (texture) glTexCoord2d((x + step) * texScale, (z + step) * texScale);
            glVertex3d(x + step, groundY + h3, z + step);
            
            // Top-left corner (x, z+step)
            h4 = terrainHeight(x, z + step, steepness);
            terrainNormal(x, z + step, steepness, &nx4, &ny4, &nz4);
            glNormal3d(nx4, ny4, nz4);
            if (texture) glTexCoord2d(x * texScale, (z + step) * texScale);
            glVertex3d(x, groundY + h4, z + step);
        }
    }
    glEnd();
    
    // Disable texturing after drawing
    if (texture)
    {
        glDisable(GL_TEXTURE_2D);
    }
    
    // Restore default specular material properties
    float white[] = {1.0, 1.0, 1.0, 1.0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32.0);
    
    // Draw normals if requested
    if (showNormals)
    {
        GLboolean wasLit = glIsEnabled(GL_LIGHTING);
        if (wasLit) glDisable(GL_LIGHTING);
        
        glColor3f(1.0, 1.0, 0.0); // Yellow normals
        const double normalLen = 0.3;
        const double normalStep = 1.0; // Sparse normals for visibility
        
        glBegin(GL_LINES);
        for (double x = -size; x <= size; x += normalStep)
        {
            for (double z = -size; z <= size; z += normalStep)
            {
                double h = terrainHeight(x, z, steepness);
                double nx, ny, nz;
                terrainNormal(x, z, steepness, &nx, &ny, &nz);
                
                glVertex3d(x, groundY + h, z);
                glVertex3d(x + nx * normalLen, groundY + h + ny * normalLen, z + nz * normalLen);
            }
        }
        glEnd();
        
        if (wasLit) glEnable(GL_LIGHTING);
    }
}

/*
 *  Draw axes
 *  Length is the axis length in world coordinates
 */
void drawAxes(double len)
{
    //  Save transformation
    glPushMatrix();

    //  Draw axes
    glBegin(GL_LINES);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(len, 0.0, 0.0);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, len, 0.0);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, 0.0, len);
    glEnd();

    //  Label axes
    glRasterPos3d(len, 0.0, 0.0);
    Print("X");
    glRasterPos3d(0.0, len, 0.0);
    Print("Y");
    glRasterPos3d(0.0, 0.0, len);
    Print("Z");

    //  Undo transformations
    glPopMatrix();
}