#version 120

uniform sampler2D colorTex;
uniform sampler2D normalTex;
uniform int fogEnabled;

varying vec3 T; // Tangent vector
varying vec3 B; // Bitangent vector
varying vec3 N; // Normal vector
varying vec3 L; // Light vector
varying vec3 V; // View vector

void main()
{
   // Orthonormal basis for tangent space
   vec3 Tn = normalize(T);
   vec3 Bn = normalize(B);
   vec3 Nn = normalize(N);
   mat3 TBN = mat3(Tn, Bn, Nn);

   // Sample normal from normal map (tangent space, stored in [0,1])
   vec2 uv = gl_TexCoord[0].st;
   vec3 nTex = texture2D(normalTex, uv).rgb * 2.0 - 1.0;
   // Flip normal Y component to match my current OpenGL texture coordinate system
   nTex.g = -nTex.g;
   // Increase normal strength by scaling the XY components before renormalizing.
   float strength = 3.0;
   nTex.xy *= strength;
   nTex = normalize(nTex);
   vec3 Np = normalize(TBN * nTex);

   // Lighting vectors
   vec3 Ld = normalize(L);
   vec3 Vd = normalize(V);

   // Diffuse and specular terms
   float Id = max(dot(Ld, Np), 0.0);
   float Is = 0.0;
   if (Id > 0.0)
   {
      vec3 R = reflect(-Ld, Np);
      Is = pow(max(dot(R, Vd), 0.0), gl_FrontMaterial.shininess);
   }

   // Base color from diffuse texture
   vec4 base = texture2D(colorTex, uv);

   // Combine with fixed-function style light products
   vec4 ambient  = gl_FrontLightProduct[0].ambient  * base;
   vec4 diffuse  = gl_FrontLightProduct[0].diffuse  * base * Id;
   vec4 specular = gl_FrontLightProduct[0].specular * Is;
   vec4 color = ambient + diffuse + specular;

   // Apply linear fog using built-in fog state so it matches enableFog()
   if (fogEnabled != 0)
   {
      /* (helped by AI) */
      float fogCoord = gl_FogFragCoord;
      float fogFactor = (gl_Fog.end - fogCoord) * gl_Fog.scale;
      fogFactor = clamp(fogFactor, 0.0, 1.0);
      color = mix(gl_Fog.color, color, fogFactor);
   }

   gl_FragColor = color;
}
