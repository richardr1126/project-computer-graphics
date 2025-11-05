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
