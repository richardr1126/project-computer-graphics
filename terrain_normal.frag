#version 120

uniform sampler2D colorTex;   // Diffuse/base color texture
uniform sampler2D normalTex;  // Normal map texture (tangent-space normals)
uniform int fogEnabled;       // Non-zero when fog should be applied

// Inputs from the vertex shader (eye space)
varying vec3 T; // Tangent vector
varying vec3 B; // Bitangent vector
varying vec3 N; // Normal vector
varying vec3 L; // Light vector
varying vec3 V; // View vector

void main()
{
   // 1) Build TBN basis in eye space
   vec3 Tn = normalize(T);
   vec3 Bn = normalize(B);
   vec3 Nn = normalize(N);
   mat3 TBN = mat3(Tn, Bn, Nn); // Columns are tangent, bitangent, normal

   // 2) Sample and unpack normal from normal map
   vec2 uv = gl_TexCoord[0].st;
   vec3 nTex = texture2D(normalTex, uv).rgb * 2.0 - 1.0;

   // Flip Y to match OpenGL texture convention
   nTex.g = -nTex.g;

   // Boost bumpiness by scaling XY components
   float strength = 3.0;
   nTex.xy *= strength;
   nTex = normalize(nTex);

   // 3) Transform tangent-space normal into eye space
   vec3 Np = normalize(TBN * nTex);

   // 4) Compute lighting vectors and Phong terms
   vec3 Ld = normalize(L); // Light direction (eye space)
   vec3 Vd = normalize(V); // View direction  (eye space)

   // Diffuse term: Lambertian dot product between light and normal
   float Id = max(dot(Ld, Np), 0.0);

   // Specular term: only when surface faces the light
   float Is = 0.0;
   if (Id > 0.0)
   {
      vec3 R = reflect(-Ld, Np); // Reflection of light about the normal
      Is = pow(max(dot(R, Vd), 0.0), gl_FrontMaterial.shininess);
   }

   // 5) Sample base color and combine with lighting
   vec4 base = texture2D(colorTex, uv);

   vec4 ambient  = gl_FrontLightProduct[0].ambient  * base;
   vec4 diffuse  = gl_FrontLightProduct[0].diffuse  * base * Id;
   vec4 specular = gl_FrontLightProduct[0].specular * Is;
   vec4 color = ambient + diffuse + specular;

   // 6) Apply linear fog (if enabled)
   if (fogEnabled != 0)
   {
      /* (helped by AI) */
      float fogCoord = gl_FogFragCoord; // Distance from eye (set in vertex shader)
      float fogFactor = (gl_Fog.end - fogCoord) * gl_Fog.scale;
      fogFactor = clamp(fogFactor, 0.0, 1.0);
      color = mix(gl_Fog.color, color, fogFactor);
   }

   // 7) Output final fragment color
   gl_FragColor = color;
}
