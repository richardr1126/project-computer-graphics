/*
 *  Recursive tree object - implementation
 */

#include "../utils.h"
#include "tree.h"

/* Deterministic pseudo-random based on integer seed */
static double rand01(unsigned int s)
{
    /* xorshift32 */
    unsigned int x = s ? s : 1u;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    /* map to 0..1 */
    return (x & 0xFFFFFFu) / 16777215.0;
}

/* Draw a textured tapered frustum (r0 -> r1) along +Y with UV controls */
static void drawFrustum(double r0, double r1, double length, unsigned int sides, unsigned int texture, double uOffset, double vScale)
{
    if (sides < 6) sides = 12;
    const double d = 360.0 / (double)sides;

    if (texture)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glColor3f(1,1,1);
    }

    /* Normal Y component for frustum: k = (r0 - r1)/length */
    const double k = (length > 0.0) ? ((r0 - r1) / length) : 0.0;

    glBegin(GL_QUAD_STRIP);
    for (double ang = 0; ang <= 360.0001; ang += d)
    {
        double c = Cos(ang), s = Sin(ang);
        /* outward normal proportional to (c, k, s) */
        double nx = c, ny = k, nz = s;
        double nl = sqrt(nx*nx + ny*ny + nz*nz);
        if (nl > 0) { nx/=nl; ny/=nl; nz/=nl; }

        glNormal3d(nx, ny, nz);
        if (texture) glTexCoord2d(uOffset + ang/360.0, vScale);
        glVertex3d(r1 * c, length, r1 * s);
        glNormal3d(nx, ny, nz);
        if (texture) glTexCoord2d(uOffset + ang/360.0, 0.0);
        glVertex3d(r0 * c, 0.0, r0 * s);
    }
    glEnd();

    if (texture)
    {
        glDisable(GL_TEXTURE_2D);
    }
}

/* Optionally draw a few normals for debugging around mid-height */
static void drawFrustumNormals(double r0, double r1, double length)
{
    const double L = 0.3; // normal line length
    glColor3f(1,1,0);
    glBegin(GL_LINES);
    for (int i=0;i<6;i++)
    {
        double ang = i * 60.0;
        double c = Cos(ang), s = Sin(ang);
        double y = length * 0.5;
        double rmid = r0 + (r1 - r0) * 0.5;
        /* normal direction same as frustum */
        double k = (length > 0.0) ? ((r0 - r1) / length) : 0.0;
        double nx = c, ny = k, nz = s;
        double nl = sqrt(nx*nx + ny*ny + nz*nz);
        if (nl > 0) { nx/=nl; ny/=nl; nz/=nl; }
        glVertex3d(rmid*c, y, rmid*s);
        glVertex3d(rmid*c + L*nx, y + L*ny, rmid*s + L*nz);
    }
    glEnd();
}

/* Recursive branch: starts at origin, grows along +Y. */
static void drawBranch(double len, double r, int depth, double swayDeg, unsigned int texture, int showNormals, unsigned int seed)
{
    if (depth <= 0 || len <= 0.05 || r <= 0.015) return;

    double taper = 0.70 + 0.15 * rand01(seed + 21u);
    double rEnd = r * taper;
    if (depth <= 2) rEnd *= 0.75;
    if (depth == 1) rEnd = fmax(0.02, rEnd * 0.5);

    glPushMatrix();

    unsigned int sides = (depth >= 4) ? 24 : (depth >= 2 ? 18 : 14);
    int segs = 3 + (len > 2.0 ? 1 : 0);
    double segLen = len / (double)segs;
    double vScale = fmax(1.0, len * 1.5);
    double uOff = rand01(seed + 100u);
    double prevR = r;
    
    /* Add slight overlap between segments to hide seams */
    double overlap = 0.015;

    for (int si=0; si<segs; ++si)
    {
        double t0 = (double)si / (double)segs;
        double t1 = (double)(si+1) / (double)segs;
        double r0 = r - (r - rEnd) * t0;
        double r1 = r - (r - rEnd) * t1;
        
        /* Extend segment slightly to overlap with next segment */
        double actualLen = (si < segs-1) ? segLen + overlap : segLen;

        drawFrustum(r0, r1, actualLen, sides, texture, uOff, vScale * (actualLen/len));
        if (showNormals)
        {
            GLboolean wasLit = glIsEnabled(GL_LIGHTING);
            if (wasLit) glDisable(GL_LIGHTING);
            drawFrustumNormals(r0, r1, actualLen);
            if (wasLit) glEnable(GL_LIGHTING);
        }

        glTranslated(0, segLen, 0);
        if (si < segs-1)
        {
            double bend = 2.0 + 3.0 * rand01(seed + 22u + si*3u);
            double bendDir = 360.0 * rand01(seed + 23u + si*7u);
            double sway = 0.8 * Sin(swayDeg + (double)(depth+si)*17.0);
            double ax = Cos(bendDir), az = Sin(bendDir);
            glRotated(bend + sway, ax, 0, az);
            uOff = fmod(uOff + 0.15 * rand01(seed + 101u + si*5u), 1.0);
        }
        prevR = r1;
    }

    /* More branches at base (higher depth), fewer at tips */
    int childCount = 2;
    if (depth >= 5) {
        /* Near base: commonly 3 branches */
        childCount = (rand01(seed*911u) < 0.7) ? 3 : 2;
    } else if (depth >= 4) {
        /* Mid-lower: sometimes 3 branches */
        childCount = (rand01(seed*911u) < 0.4) ? 3 : 2;
    } else if (depth >= 2) {
        /* Upper branches: mostly 2, occasionally 3 */
        childCount = 2 + (int)(1.5 * rand01(seed*911u) + 0.3);
    }
    double baseAngleOffset = 360.0 * rand01(seed*713u);
    for (int i=0; i<childCount; ++i)
    {
        unsigned int cseed = seed * 131u + (unsigned int)i * 977u + depth * 37u;
        double angleSpacing = 360.0 / (double)childCount;
        double angY = baseAngleOffset + i * angleSpacing + (30.0 * rand01(cseed + 1u) - 15.0);
        double verticalBias = (depth <= 2) ? 8.0 : 0.0;
        double tilt = 25.0 + 10.0 * rand01(cseed + 2u) - verticalBias;
        double scale = 0.70 + 0.18 * rand01(cseed + 3u);
        double swayYaw = 2.5 * Sin(swayDeg + (double)(i*depth)*13.0);

        glPushMatrix();
        glRotated(angY + swayYaw, 0,1,0);
        glRotated(-tilt, 1,0,0);

        /* Transition collar: start child near parent radius to hide edge */
        double childLen = len * scale;
        double joinR = prevR;
        if (joinR > 0.001 && childLen > 0.05)
        {
            double uOffC = rand01(cseed + 200u);

            if (depth > 1)
            {
                /* Usual case: add a short adapter then recurse */
                double childBaseR = joinR * (0.88 + 0.07 * rand01(cseed + 4u)); /* 0.88..0.95 */
                double adapterLen = fmin(childLen * 0.22, 0.35);
                drawFrustum(joinR * 0.98, childBaseR, adapterLen, sides, texture, uOffC, fmax(1.0, adapterLen * 1.5));
                if (showNormals)
                {
                    GLboolean wasLit = glIsEnabled(GL_LIGHTING);
                    if (wasLit) glDisable(GL_LIGHTING);
                    drawFrustumNormals(joinR * 0.98, childBaseR, adapterLen);
                    if (wasLit) glEnable(GL_LIGHTING);
                }
                glTranslated(0, adapterLen, 0);
                double remain = childLen - adapterLen;
                if (remain > 0.05)
                {
                    drawBranch(remain, childBaseR, depth - 1, swayDeg, texture, showNormals, cseed);
                }
            }
            else
            {
                /* Terminal tip: draw a full-length twig instead of only an adapter */
                double tipLen = childLen;
                double tipR  = fmax(0.004, joinR * 0.35);
                drawFrustum(joinR * 0.98, tipR, tipLen, sides, texture, uOffC, fmax(1.0, tipLen * 1.5));
                if (showNormals)
                {
                    GLboolean wasLit = glIsEnabled(GL_LIGHTING);
                    if (wasLit) glDisable(GL_LIGHTING);
                    drawFrustumNormals(joinR * 0.98, tipR, tipLen);
                    if (wasLit) glEnable(GL_LIGHTING);
                }
            }
        }
        glPopMatrix();
    }

    glPopMatrix();
}

/* Simple helper to compute approximate terrain height used in ground.c for placement */
static double approxTerrainY(double x, double z)
{
    /* Mirror ground.c: steepness=0.5 and groundY=-3.0 */
    double steep = 0.5;
    double h = 0.0;
    h += 0.3 * sin(x * 0.5) * cos(z * 0.5);
    h += 0.2 * sin(x * 0.8 + z * 0.3);
    h += 0.15 * cos(x * 1.2 - z * 0.7);
    return -3.0 + h * steep;
}

/* Draw a single tree at world position (x,z), rooted on terrain */
static void drawTreeAt(double x, double z, double anim, unsigned int texture, int showNormals, unsigned int seed)
{
    double y = approxTerrainY(x, z);
    double baseLen = 2.5 + 1.2 * rand01(seed + 5u);
    double baseRad = 0.13 + 0.05 * rand01(seed + 6u);
    int depth = 4 + (int)(2.0 * rand01(seed + 7u));

    glPushMatrix();
    glTranslated(x, y, z);
    /* Small base tilt to avoid perfect verticals */
    double tilt = 2.0 * (rand01(seed+11u) - 0.5);
    double tiltDir = 360.0 * rand01(seed+12u);
    glRotated(tiltDir, 0,1,0);
    glRotated(tilt, 1,0,0);

    /* Base flare before main trunk for more natural look */
    double flareLen = 0.35;
    double flareR0  = baseRad * 1.25;
    double flareR1  = baseRad;
    double uOffFlare = rand01(seed + 200u);
    drawFrustum(flareR0, flareR1, flareLen, 24, texture, uOffFlare, fmax(1.0, flareLen * 1.5));
    if (showNormals)
    {
        GLboolean wasLit = glIsEnabled(GL_LIGHTING);
        if (wasLit) glDisable(GL_LIGHTING);
        drawFrustumNormals(flareR0, flareR1, flareLen);
        if (wasLit) glEnable(GL_LIGHTING);
    }
    glTranslated(0, flareLen, 0);
    baseLen = (baseLen > flareLen) ? (baseLen - flareLen) : baseLen;

    drawBranch(baseLen, baseRad, depth, anim, texture, showNormals, seed);
    glPopMatrix();
}

/* Public entry: draw a ring (or two) of trees around origin/bullseyes */
void drawTreeScene(double anim, int showNormals, unsigned int texture)
{
    /* Material: slightly less specular for bark */
    float spec[] = {0.05f,0.05f,0.05f,1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 6.0f);

    /* Four rings of trees with randomized positions for less uniformity */
    const double r1 = 15.0;
    const double r2 = 22.0;
    const double r3 = 29.0;
    const double r4 = 36.0;

    /* Ring 1 */
    int n1 = 8;
    for (int i=0;i<n1;i++)
    {
        unsigned int seed = 12345u + (unsigned int)i*17u;
        double a = i * (360.0 / n1) + (25.0 * rand01(seed + 50u) - 12.5);
        double rVar = r1 + (3.0 * rand01(seed + 51u) - 1.5);
        double x = rVar * Cos(a);
        double z = rVar * Sin(a);
        drawTreeAt(x, z, anim, texture, showNormals, seed);
    }
    /* Ring 2 */
    int n2 = 12;
    for (int i=0;i<n2;i++)
    {
        unsigned int seed = 67890u + (unsigned int)i*31u;
        double a = i * (360.0 / n2) + 12.0 + (20.0 * rand01(seed + 50u) - 10.0);
        double rVar = r2 + (3.5 * rand01(seed + 51u) - 1.75);
        double x = rVar * Cos(a);
        double z = rVar * Sin(a);
        drawTreeAt(x, z, anim, texture, showNormals, seed);
    }
    /* Ring 3 */
    int n3 = 16;
    for (int i=0;i<n3;i++)
    {
        unsigned int seed = 24680u + (unsigned int)i*41u;
        double a = i * (360.0 / n3) + 8.0 + (18.0 * rand01(seed + 50u) - 9.0);
        double rVar = r3 + (4.0 * rand01(seed + 51u) - 2.0);
        double x = rVar * Cos(a);
        double z = rVar * Sin(a);
        drawTreeAt(x, z, anim, texture, showNormals, seed);
    }
    /* Ring 4 */
    int n4 = 20;
    for (int i=0;i<n4;i++)
    {
        unsigned int seed = 13579u + (unsigned int)i*53u;
        double a = i * (360.0 / n4) + 15.0 + (16.0 * rand01(seed + 50u) - 8.0);
        double rVar = r4 + (4.5 * rand01(seed + 51u) - 2.25);
        double x = rVar * Cos(a);
        double z = rVar * Sin(a);
        drawTreeAt(x, z, anim, texture, showNormals, seed);
    }

    /* Restore generic specular */
    float white[] = {1,1,1,1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32.0f);
}
