/*
 *  Ground terrain object - implementation file
 *  Contains drawing functions for terrain
 */

#include "ground.h"
#include "../utils.h"

// Smoothstep helper
static inline double smoothstep01(double t)
{
    if (t <= 0.0) return 0.0;
    if (t >= 1.0) return 1.0;
    return t * t * (3.0 - 2.0 * t);
}

static inline double lerp(double a, double b, double t)
{
    return a + t * (b - a);
}

// Fast 2D integer hash -> [0,1]
static inline double hash2i(int x, int y)
{
    unsigned int h = (unsigned int)(x) * 374761393u + (unsigned int)(y) * 668265263u; // large primes
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= (h >> 16);
    return (h & 0xFFFFFFu) / 16777215.0; // 24-bit to [0,1]
}

// Value noise 2D with smooth interpolation, returns in [-1,1]
static double valueNoise2(double x, double y)
{
    int ix = (int)floor(x);
    int iy = (int)floor(y);
    double fx = x - ix;
    double fy = y - iy;

    double u = smoothstep01(fx);
    double v = smoothstep01(fy);

    double n00 = hash2i(ix + 0, iy + 0);
    double n10 = hash2i(ix + 1, iy + 0);
    double n01 = hash2i(ix + 0, iy + 1);
    double n11 = hash2i(ix + 1, iy + 1);

    double nx0 = lerp(n00, n10, u);
    double nx1 = lerp(n01, n11, u);
    double nxy = lerp(nx0, nx1, v);
    return 2.0 * nxy - 1.0; // map to [-1,1]
}

// Fractal Brownian Motion (fBm)
static double fbm2(double x, double y, int octaves, double lacunarity, double gain)
{
    double sum = 0.0;
    double amp = 0.5;
    double freq = 1.0;
    for (int i = 0; i < octaves; ++i)
    {
        sum += amp * valueNoise2(x * freq, y * freq);
        freq *= lacunarity;
        amp *= gain;
    }
    return sum; // roughly in [-1,1]
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
 *  Helper to compute normal from finite difference heights
 */
static void computeFiniteDiffNormal(double hL, double hR, double hD, double hU, double delta, double* nx, double* ny, double* nz)
{
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
    double len = Sqrt(cx * cx + cy * cy + cz * cz);
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
    
    computeFiniteDiffNormal(hL, hR, hD, hU, delta, nx, ny, nz);
}

/*
 *  Draw ground terrain with varied height
 *  Optimized: caches a display list for the static mesh to avoid per-frame recomputation
 */
void drawGround(double steepness, double size, double groundY, unsigned int texture, int showNormals)
{
    const double step = 0.5;  // Grid resolution
    const double texScale = 0.2; // Texture coordinate scale
    const double radius2 = size * size; // Island radius squared

    static GLuint groundList = 0;
    static double cachedSteep = 0.0, cachedSize = 0.0, cachedY = 0.0;
    static unsigned int cachedTex = 0;

    int needsRebuild = 0;
    if (!groundList) needsRebuild = 1;
    if (cachedSteep != steepness || cachedSize != size || cachedY != groundY || cachedTex != texture)
        needsRebuild = 1;

    if (needsRebuild)
    {
        if (groundList) glDeleteLists(groundList, 1);
        groundList = glGenLists(1);

        // Precompute heights and normals at grid vertices
        int nx = (int)floor((2.0*size)/step) + 1;
        int nz = (int)floor((2.0*size)/step) + 1;
        int total = nx * nz;
        double* H = (double*)malloc(sizeof(double)*total);
        double* NX = (double*)malloc(sizeof(double)*total);
        double* NY = (double*)malloc(sizeof(double)*total);
        double* NZ = (double*)malloc(sizeof(double)*total);
        if (!H || !NX || !NY || !NZ)
        {
            free(H); free(NX); free(NY); free(NZ);
            // Fallback to immediate path if allocation fails
        }

        double x0 = -size;
        double z0 = -size;
        for (int iz=0; iz<nz; ++iz)
        {
            double z = z0 + iz*step;
            for (int ix=0; ix<nx; ++ix)
            {
                double x = x0 + ix*step;
                int idx = iz*nx + ix;
                H[idx] = terrainHeight(x, z, steepness);
                double nxv, nyv, nzv;
                terrainNormal(x, z, steepness, &nxv, &nyv, &nzv);
                NX[idx] = nxv; NY[idx] = nyv; NZ[idx] = nzv;
            }
        }

        glNewList(groundList, GL_COMPILE);
        // Set material properties for ground - minimal specular to avoid stretching artifacts
        float groundSpecular[] = {0.05f, 0.05f, 0.05f, 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, groundSpecular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 2.0f);

        if (texture)
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glColor3f(1.0f, 1.0f, 1.0f);
        }
        else
        {
            glColor3f(0.3f, 0.5f, 0.2f);
        }

        // Render as triangle strips per row, clipped to a circular island
        for (int iz=0; iz<nz-1; ++iz)
        {
            double zA = z0 + iz*step;
            double zB = z0 + (iz+1)*step;
            int segmentOpen = 0;
            for (int ix=0; ix<nx; ++ix)
            {
                double x = x0 + ix*step;
                int iA = iz*nx + ix;
                int iB = (iz+1)*nx + ix;

                int inA = (x*x + zA*zA) <= radius2;
                int inB = (x*x + zB*zB) <= radius2;

                if (inA && inB)
                {
                    if (!segmentOpen)
                    {
                        glBegin(GL_TRIANGLE_STRIP);
                        segmentOpen = 1;
                    }

                    glNormal3d(NX[iA], NY[iA], NZ[iA]);
                    if (texture) glTexCoord2d(x * texScale, zA * texScale);
                    glVertex3d(x, groundY + H[iA], zA);

                    glNormal3d(NX[iB], NY[iB], NZ[iB]);
                    if (texture) glTexCoord2d(x * texScale, zB * texScale);
                    glVertex3d(x, groundY + H[iB], zB);
                }
                else if (segmentOpen)
                {
                    glEnd();
                    segmentOpen = 0;
                }
            }
            if (segmentOpen) glEnd();
        }

        if (texture) glDisable(GL_TEXTURE_2D);

        // Restore default specular
        float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32.0f);
        glEndList();

        // Cache params used
        cachedSteep = steepness; cachedSize = size; cachedY = groundY; cachedTex = texture;

        free(H); free(NX); free(NY); free(NZ);
    }

    // Call the cached list
    if (groundList)
        glCallList(groundList);

    // Draw normals if requested (dynamic, not in the list)
    if (showNormals)
    {
        GLboolean wasLit = glIsEnabled(GL_LIGHTING);
        if (wasLit) glDisable(GL_LIGHTING);

        glColor3f(1.0f, 1.0f, 0.0f);
        const double normalLen = 0.3;
        const double normalStep = 1.0;

        glBegin(GL_LINES);
        for (double x = -size; x <= size; x += normalStep)
        {
            for (double z = -size; z <= size; z += normalStep)
            {
                if ((x*x + z*z) > radius2) continue; // Only draw normals inside island
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
 *  Compute bowl-like mountain height between inner and outer radii
 *  Produces low heights at innerR and rises toward outerR with noise
 */
static double mountainHeight(double x, double z, double innerR, double outerR, double heightScale)
{
    // Radial profile and envelope across the ring
    double r = Sqrt(x*x + z*z);
    if (r <= innerR) return 0.0;
    if (r >= outerR) r = outerR;

    double s = (r - innerR) / (outerR - innerR); // 0..1 across the ring
    if (s < 0.0) s = 0.0; if (s > 1.0) s = 1.0;
    
    // Basic shape: rise from inner rim, peak, then fall to outer rim
    // Use a sine wave for the base shape
    double base = sin(s * 3.14159); 
    
    // Add some noise for variation
    double noise = fbm2(x * 0.1, z * 0.1, 4, 2.0, 0.5);
    
    // Combine base shape with noise
    double mountain = base * (0.5 + 0.5 * noise);
    
    // Sink near the inner rim to avoid cracks/z-fighting under the forest ground
    double innerBlend = smoothstep01(s / 0.12); // 0 at rim, 1 after ~12% of the band
    double worldH = heightScale * mountain;
    worldH -= 0.6 * (1.0 - innerBlend); // sink up to 0.6 world units at the seam
    
    return worldH;
}

/*
 *  Finite-difference normal for mountain surface
 */
static void mountainNormal(double x, double z, double innerR, double outerR, double heightScale,
                           double* nx, double* ny, double* nz)
{
    const double d = 0.2;
    double hL = mountainHeight(x - d, z, innerR, outerR, heightScale);
    double hR = mountainHeight(x + d, z, innerR, outerR, heightScale);
    double hD = mountainHeight(x, z - d, innerR, outerR, heightScale);
    double hU = mountainHeight(x, z + d, innerR, outerR, heightScale);

    computeFiniteDiffNormal(hL, hR, hD, hU, d, nx, ny, nz);
}

/*
 *  Draw a circular mountain ring (bowl-like) surrounding the ground island
 */
void drawMountainRing(double innerR, double outerR, double baseY,
                      unsigned int texture, int showNormals, double heightScale)
{
    if (outerR <= innerR) return;

    // Balanced step for detail vs performance over a vast area
    const double step = 1.0;
    const double texScale = 0.08; // texture tiling
    const double innerR2 = innerR*innerR;
    const double outerR2 = outerR*outerR;

    static GLuint ringList = 0;
    static double cInner = 0.0, cOuter = 0.0, cBaseY = 0.0, cHeight = 0.0;
    static unsigned int cTex = 0;

    int rebuild = 0;
    if (!ringList) rebuild = 1;
    if (cInner != innerR || cOuter != outerR || cBaseY != baseY || cHeight != heightScale || cTex != texture)
        rebuild = 1;

    if (rebuild)
    {
        if (ringList) glDeleteLists(ringList,1);
        ringList = glGenLists(1);

        // Grid bounds cover the whole outer disk
        int nx = (int)floor((2.0*outerR)/step) + 1;
        int nz = (int)floor((2.0*outerR)/step) + 1;
        double x0 = -outerR;
        double z0 = -outerR;

        glNewList(ringList, GL_COMPILE);

        // Subtle specular to avoid harsh highlights on large surfaces
        float spec[] = {0.04f, 0.04f, 0.04f, 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 4.0f);

        if (texture)
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glColor3f(1.0f, 1.0f, 1.0f);
        }
        else
        {
            glColor3f(0.35f, 0.35f, 0.35f);
        }

        // Triangle strips per row, but only emit segments within the ring [innerR, outerR]
        for (int iz=0; iz<nz-1; ++iz)
        {
            double zA = z0 + iz*step;
            double zB = z0 + (iz+1)*step;
            int open = 0;
            for (int ix=0; ix<nx; ++ix)
            {
                double x = x0 + ix*step;
                double r2A = x*x + zA*zA;
                double r2B = x*x + zB*zB;
                int inA = (r2A >= innerR2) && (r2A <= outerR2);
                int inB = (r2B >= innerR2) && (r2B <= outerR2);

                if (inA && inB)
                {
                    if (!open)
                    {
                        glBegin(GL_TRIANGLE_STRIP);
                        open = 1;
                    }

                    double nxA, nyA, nzA; mountainNormal(x, zA, innerR, outerR, heightScale, &nxA, &nyA, &nzA);
                    double hA = mountainHeight(x, zA, innerR, outerR, heightScale);
                    glNormal3d(nxA, nyA, nzA);
                    if (texture) glTexCoord2d(x * texScale, zA * texScale);
                    glVertex3d(x, baseY + hA, zA);

                    double nxB, nyB, nzB; mountainNormal(x, zB, innerR, outerR, heightScale, &nxB, &nyB, &nzB);
                    double hB = mountainHeight(x, zB, innerR, outerR, heightScale);
                    glNormal3d(nxB, nyB, nzB);
                    if (texture) glTexCoord2d(x * texScale, zB * texScale);
                    glVertex3d(x, baseY + hB, zB);
                }
                else if (open)
                {
                    glEnd();
                    open = 0;
                }
            }
            if (open) glEnd();
        }

        if (texture) glDisable(GL_TEXTURE_2D);

        // Restore default specular
        float white[] = {1.0f,1.0f,1.0f,1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32.0f);

        glEndList();

        cInner = innerR; cOuter = outerR; cBaseY = baseY; cHeight = heightScale; cTex = texture;
    }

    if (ringList)
        glCallList(ringList);

    if (showNormals)
    {
        GLboolean wasLit = glIsEnabled(GL_LIGHTING);
        if (wasLit) glDisable(GL_LIGHTING);

        glColor3f(1.0f, 0.8f, 0.0f);
        const double nLen = 0.8;
        const double nStep = 6.0;
        glBegin(GL_LINES);
        for (double x = -outerR; x <= outerR; x += nStep)
        {
            for (double z = -outerR; z <= outerR; z += nStep)
            {
                double r2 = x*x + z*z;
                if (r2 < innerR2 || r2 > outerR2) continue;
                double nx, ny, nz;
                double h = mountainHeight(x, z, innerR, outerR, heightScale);
                mountainNormal(x, z, innerR, outerR, heightScale, &nx, &ny, &nz);
                glVertex3d(x, baseY + h, z);
                glVertex3d(x + nx*nLen, baseY + h + ny*nLen, z + nz*nLen);
            }
        }
        glEnd();

        if (wasLit) glEnable(GL_LIGHTING);
    }
}
