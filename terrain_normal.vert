#version 120

varying vec3 T; // Tangent vector
varying vec3 B; // Bitangent vector
varying vec3 N; // Normal vector
varying vec3 L; // Light vector
varying vec3 V; // View vector

void main()
{
   // Vertex position and normal in eye space
   vec3 P  = vec3(gl_ModelViewMatrix * gl_Vertex);
   vec3 N0 = normalize(gl_NormalMatrix * gl_Normal);

   // Build a tangent basis from the normal and an up-like vector
   vec3 up = (abs(N0.y) < 0.999) ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
   vec3 T0 = normalize(cross(up, N0));
   vec3 B0 = cross(N0, T0);

   // Light and view vectors in eye space
   vec3 LightPos = vec3(gl_LightSource[0].position);
   vec3 L0 = LightPos - P;
   vec3 V0 = -P;

   T = T0;
   B = B0;
   N = N0;
   L = L0;
   V = V0;

   // Fog coordinate: distance from eye in eye space
   gl_FogFragCoord = length(P);

   // Pass through texture coordinates
   gl_TexCoord[0] = gl_MultiTexCoord0;

   // Standard transformed position
   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
