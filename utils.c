#include "utils.h"

/*
 *  Print message to stderr and exit
 *  Original author: Willem A. (Vlakkies) Schreuder
 *  @param format format string
 *  @param ... arguments
 */
void Fatal(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  exit(1);
}

/*
 *  Check for OpenGL errors and print to stderr
 *  Original author: Willem A. (Vlakkies) Schreuder
 *  @param where location of error
 */
void ErrCheck(const char *where) {
  int err = glGetError();
  if (err)
    fprintf(stderr, "ERROR: %s [%s]\n", gluErrorString(err), where);
}

#define LEN 8192 //  Maximum length of text string
/*
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible
 *  Original author: Willem A. (Vlakkies) Schreuder
 *  @param format format string
 *  @param ... arguments
 */
void Print(const char *format, ...) {
  char buf[LEN];
  char *ch = buf;
  va_list args;
  //  Turn the parameters into a character string
  va_start(args, format);
  vsnprintf(buf, LEN, format, args);
  va_end(args);
  //  Display the characters one at a time at the current raster position
  while (*ch) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *ch++);
}

/*
 *  Length of a 3D vector.
 *  @param x x component of vector
 *  @param y y component of vector
 *  @param z z component of vector
 */
double Vec3Length(double x, double y, double z) { return sqrt(x * x + y * y + z * z); }

/*
 *  Normalize vector in place if it has non-negligible length.
 *  @param x x component of vector
 *  @param y y component of vector
 *  @param z z component of vector
 */
void Vec3Normalize(double *x, double *y, double *z) {
  double len = Vec3Length(*x, *y, *z);
  if (len > 1e-12) {
    *x /= len;
    *y /= len;
    *z /= len;
  }
}

/*
 *  Cross product: result = a x b.
 *  @param ax x component of a
 *  @param ay y component of a
 *  @param az z component of a
 *  @param bx x component of b
 *  @param by y component of b
 *  @param bz z component of b
 *  @param rx x component of result
 *  @param ry y component of result
 *  @param rz z component of result
 */
void Vec3Cross(double ax, double ay, double az, double bx, double by, double bz,
               double *rx, double *ry, double *rz) {
  if (rx)
    *rx = ay * bz - az * by;
  if (ry)
    *ry = az * bx - ax * bz;
  if (rz)
    *rz = ax * by - ay * bx;
}

/*
 *  Convert spherical angles (degrees) to a direction vector pointing in the
 *  same direction used by the camera/arrow math.
 *  @param th azimuth angle
 *  @param ph elevation angle
 *  @param dx x component of direction vector
 *  @param dy y component of direction vector
 *  @param dz z component of direction vector
 */
void DirectionFromAngles(double th, double ph,
                         double *dx, double *dy, double *dz) {
  if (dx)
    *dx = Sin(th) * Cos(ph);
  if (dy)
    *dy = Sin(ph);
  if (dz)
    *dz = -Cos(th) * Cos(ph);
}

/*
 *  Fast deterministic random number in [0,1] from integer seed.
 *  @param seed input seed
 */
double Rand01(unsigned int seed) {
  unsigned int x = seed ? seed : 1u;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return (x & 0xFFFFFFu) / 16777215.0;
}

/*
 *  Reverse n bytes
 *  Original author: Willem A. (Vlakkies) Schreuder
 *  @param x pointer to bytes
 *  @param n number of bytes
 */
static void Reverse(void *x, const int n) {
  char *ch = (char *)x;
  for (int k = 0; k < n / 2; k++) {
    char tmp = ch[k];
    ch[k] = ch[n - 1 - k];
    ch[n - 1 - k] = tmp;
  }
}

/*
 *  Load texture from BMP file
 *  Original author: Willem A. (Vlakkies) Schreuder
 *  Extended with 32-bit support by Richard Roberson
 *  @param file name of file
 *  @return texture name
 */
unsigned int LoadTexBMP(const char *file) {
  //  Open file
  FILE *f = fopen(file, "rb");
  if (!f)
    Fatal("Cannot open file %s\n", file);
  //  Check image magic
  unsigned short magic;
  if (fread(&magic, 2, 1, f) != 1)
    Fatal("Cannot read magic from %s\n", file);
  if (magic != 0x4D42 && magic != 0x424D)
    Fatal("Image magic not BMP in %s\n", file);
  //  Read header
  unsigned int dx, dy, off, k; // Image dimensions, offset and compression
  unsigned short nbp, bpp;     // Planes and bits per pixel
  if (fseek(f, 8, SEEK_CUR) || fread(&off, 4, 1, f) != 1 ||
      fseek(f, 4, SEEK_CUR) || fread(&dx, 4, 1, f) != 1 ||
      fread(&dy, 4, 1, f) != 1 || fread(&nbp, 2, 1, f) != 1 ||
      fread(&bpp, 2, 1, f) != 1 || fread(&k, 4, 1, f) != 1)
    Fatal("Cannot read header from %s\n", file);
  //  Reverse bytes on big endian hardware (detected by backwards magic)
  if (magic == 0x424D) {
    Reverse(&off, 4);
    Reverse(&dx, 4);
    Reverse(&dy, 4);
    Reverse(&nbp, 2);
    Reverse(&bpp, 2);
    Reverse(&k, 4);
  }
  //  Check image parameters
  unsigned int max;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, (int *)&max);
  if (dx < 1 || dx > max)
    Fatal("%s image width %d out of range 1-%d\n", file, dx, max);
  if (dy < 1 || dy > max)
    Fatal("%s image height %d out of range 1-%d\n", file, dy, max);
  if (nbp != 1)
    Fatal("%s bit planes is not 1: %d\n", file, nbp);
  if (bpp != 24 && bpp != 32)
    Fatal("%s bits per pixel is not 24 or 32: %d\n", file, bpp);
  //  32-bit BMPs often use BI_BITFIELDS (k=3) which is acceptable for RGBA
  if (k != 0 && !(bpp == 32 && k == 3))
    Fatal("%s compressed files not supported (compression=%d)\n", file, k);
#ifndef GL_VERSION_2_0
  //  OpenGL 2.0 lifts the restriction that texture size must be a power of two
  for (k = 1; k < dx; k *= 2)
    ;
  if (k != dx)
    Fatal("%s image width not a power of two: %d\n", file, dx);
  for (k = 1; k < dy; k *= 2)
    ;
  if (k != dy)
    Fatal("%s image height not a power of two: %d\n", file, dy);
#endif

  //  Determine bytes per pixel
  int bytesPerPixel = bpp / 8;
  int hasAlpha = (bpp == 32);

  //  Allocate image memory
  unsigned int size = bytesPerPixel * dx * dy;
  unsigned char *image = (unsigned char *)malloc(size);
  if (!image)
    Fatal("Cannot allocate %d bytes of memory for image %s\n", size, file);
  //  Seek to and read image
  if (fseek(f, off, SEEK_SET) || fread(image, size, 1, f) != 1)
    Fatal("Error reading data from image %s\n", file);
  fclose(f);

  //  Reverse colors (BGR -> RGB or BGRA -> RGBA)
  for (k = 0; k < size; k += bytesPerPixel) {
    unsigned char temp = image[k];
    image[k] = image[k + 2];
    image[k + 2] = temp;
  }

  //  Sanity check
  ErrCheck("LoadTexBMP");
  //  Generate 2D texture
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  //  Copy image
  if (hasAlpha) {
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, dx, dy, GL_RGBA, GL_UNSIGNED_BYTE, image);
  } else {
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, dx, dy, GL_RGB, GL_UNSIGNED_BYTE, image);
  }
  if (glGetError())
    Fatal("Error in gluBuild2DMipmaps %s %dx%d\n", file, dx, dy);
  //  Use linear filtering for magnification
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //  Use bilinear mipmapping for minification (sharper, eliminates graininess)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

  //  Enable anisotropic filtering if available (reduces blur at oblique angles)
  if (strstr((const char *)glGetString(GL_EXTENSIONS), "GL_EXT_texture_filter_anisotropic")) {
    GLfloat maxAniso;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
  }

  //  Free image memory
  free(image);
  //  Return texture name
  return texture;
}

/*
 *  Read text file into a newly allocated buffer.
 *  Caller must free the returned pointer.
 *  Original author: Willem A. (Vlakkies) Schreuder
 */
static char *ReadText(const char *file) {
  char *buffer;
  FILE *f = fopen(file, "rt");
  if (!f)
    Fatal("Cannot open text file %s\n", file);
  fseek(f, 0, SEEK_END);
  long n = ftell(f);
  rewind(f);
  buffer = (char *)malloc(n + 1);
  if (!buffer)
    Fatal("Cannot allocate %ld bytes for text file %s\n", n + 1, file);
  if (fread(buffer, n, 1, f) != 1)
    Fatal("Cannot read %ld bytes for text file %s\n", n, file);
  buffer[n] = 0;
  fclose(f);
  return buffer;
}

/*
 *  Print shader compile log if there are messages.
 */
static void PrintShaderLog(GLuint shader, const char *file) {
  GLint len = 0;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
  if (len > 1) {
    char *log = (char *)malloc(len);
    if (!log) return;
    glGetShaderInfoLog(shader, len, NULL, log);
    fprintf(stderr, "Shader log for %s:\n%s\n", file, log);
    free(log);
  }
}

/*
 *  Print program link log if there are messages.
 */
static void PrintProgramLog(GLuint prog) {
  GLint len = 0;
  glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
  if (len > 1) {
    char *log = (char *)malloc(len);
    if (!log) return;
    glGetProgramInfoLog(prog, len, NULL, log);
    fprintf(stderr, "Program link log:\n%s\n", log);
    free(log);
  }
}

/*
 *  Create Shader
 *  Original author: Willem A. (Vlakkies) Schreuder
 *  @param type shader type
 *  @param file name of file
 */
static unsigned int CreateShader(GLenum type, const char *file) {
  GLuint shader = glCreateShader(type);
  char *source = ReadText(file);
  glShaderSource(shader, 1, (const char *const *)&source, NULL);
  free(source);
  fprintf(stderr, "Compile %s\n", file);
  glCompileShader(shader);
  PrintShaderLog(shader, file);
  return shader;
}

/*
 *  Create Shader Program
 *  Original author: Willem A. (Vlakkies) Schreuder
 *  @param VertFile vertex shader file
 *  @param FragFile fragment shader file
 */
unsigned int CreateShaderProg(const char *VertFile, const char *FragFile) {
  GLuint prog = glCreateProgram();
  GLuint vert = CreateShader(GL_VERTEX_SHADER, VertFile);
  GLuint frag = CreateShader(GL_FRAGMENT_SHADER, FragFile);
  glAttachShader(prog, vert);
  glAttachShader(prog, frag);
  glLinkProgram(prog);
  PrintProgramLog(prog);
  return prog;
}
