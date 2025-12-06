#version 120

// Outputs to the fragment shader (eye space)
varying vec3 T; // Tangent vector
varying vec3 B; // Bitangent vector
varying vec3 N; // Normal vector
varying vec3 L; // Light vector   (from point to light)
varying vec3 V; // View vector    (from point to eye)

void main()
{
   // 1) Transform vertex position and normal to eye space
   vec3 P  = vec3(gl_ModelViewMatrix * gl_Vertex);      // Position in eye space
   vec3 N0 = normalize(gl_NormalMatrix * gl_Normal);    // Normal  in eye space

   // 2) Construct tangent and bitangent for TBN basis
   vec3 up = (abs(N0.y) < 0.999) ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
   vec3 T0 = normalize(cross(up, N0)); // Tangent
   vec3 B0 = cross(N0, T0);            // Bitangent (already perpendicular)

   // 3) Build light and view vectors in eye space
   vec3 LightPos = vec3(gl_LightSource[0].position); // Light position (eye space)
   vec3 L0 = LightPos - P; // From point to light
   vec3 V0 = -P;           // From point to eye (eye is at origin)

   // 4) Pass per-vertex data to the fragment shader
   T = T0;
   B = B0;
   N = N0;
   L = L0;
   V = V0;

   // 5) Compute fog coordinate (distance from eye)
   gl_FogFragCoord = length(P);

   // 6) Pass through base texture coordinates
   gl_TexCoord[0] = gl_MultiTexCoord0;

   // 7) Compute final clip-space position
   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
