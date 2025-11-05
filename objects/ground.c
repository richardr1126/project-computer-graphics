/*
 *  Ground terrain object - implementation file
 *  Contains drawing functions for terrain
 */

#include "../utils.h"
#include "ground.h"

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
 *  Optimized: caches a display list for the static mesh to avoid per-frame recomputation
 */
void drawGround(double steepness, double size, double groundY, unsigned int texture, int showNormals)
{
    const double step = 0.5;  // Grid resolution
    const double texScale = 0.2; // Texture coordinate scale

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

        // Render as triangle strips per row for fewer vertices submitted
        for (int iz=0; iz<nz-1; ++iz)
        {
            double zA = z0 + iz*step;
            double zB = z0 + (iz+1)*step;
            glBegin(GL_TRIANGLE_STRIP);
            for (int ix=0; ix<nx; ++ix)
            {
                double x = x0 + ix*step;
                int iA = iz*nx + ix;
                int iB = (iz+1)*nx + ix;

                glNormal3d(NX[iB], NY[iB], NZ[iB]);
                if (texture) glTexCoord2d(x * texScale, zB * texScale);
                glVertex3d(x, groundY + H[iB], zB);

                glNormal3d(NX[iA], NY[iA], NZ[iA]);
                if (texture) glTexCoord2d(x * texScale, zA * texScale);
                glVertex3d(x, groundY + H[iA], zA);
            }
            glEnd();
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
