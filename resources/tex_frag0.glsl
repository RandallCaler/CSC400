#version 330 core
uniform sampler2D Texture0;
uniform sampler2D shadowDepth;

out vec4 Outcolor;


uniform vec3 lightColor;
uniform vec3 albedo;
uniform vec3 emissivity;
uniform vec3 reflectance;
uniform float roughness;


in OUT_struct {
   vec3 fPos;
   vec3 fragNor;
   vec2 vTexCoord;
   vec4 fPosLS;
   vec3 vColor;
} in_struct;

/* returns 1 if shadowed */
/* called with the point projected into the light's coordinate space */
float TestShadow(vec4 LSfPos) {

  //0.005 * tan (acos(nDotl)) is better/more precise
  float depth_buffer = 0.009;

	//1: shift the coordinates from -1, 1 to 0, 1
  vec3 fLS = (vec3(LSfPos) + vec3(1.0)) * 0.5;

	//2: read off the stored depth (.) from the ShadowDepth, using the shifted.xy 
  float depth = texture(shadowDepth, fLS.xy).r;

	//3: compare to the current depth (.z) of the projected depth
  if (fLS.z > depth + depth_buffer)
    return 1.0;
  return 0.0;

	//4: return 1 if the point is shadowed

}


//Fresnel Schlick function for specular
vec3 F (vec3 F0, vec3 V, vec3 N) {
	return F0 + (vec3(1.0) - F0) * pow(1 - max(0, dot(V, N)), 5.0);
}



void main() {

  float Shade;
  float amb = 0.3;

  vec4 BaseColor = vec4(in_struct.vColor, 1);
  vec4 texColor0 = texture(Texture0, in_struct.vTexCoord);

  Shade = TestShadow(in_struct.fPosLS);

  Outcolor = amb*(texColor0) + (1.0-Shade)*texColor0*BaseColor;
}






/*#version 330 core 

out vec4 color;

// reflective properties of material
uniform sampler2D Texture0;
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float MatShine;
uniform int flip;

in vec2 vTexCoord; // texture data
in vec3 fragNor; // vertex normal in view space
in vec3 LDir; // light source vector in view space
in vec3 EPos; // vertex position in view space

void main() {
	// texture color at this location
	vec4 texColor = texture(Texture0, vTexCoord);
	// normal unit vector to vertex geometry
	vec3 normal = normalize(fragNor);
	// unit vector toward light source
	vec3 light = normalize(LDir);
	// unit vector angularly halfway between light and camera
	vec3 halfway = normalize(light - normalize(EPos));
	
	// diffusion coefficient
	float diffuse = max(0, dot(normal, light));
	// specular coefficient
	float specular = pow(max(0, dot(normal, halfway)), MatShine);
  if (flip > 0) {
    //diffuse = floor(diffuse * 4 + 4) / 8.7;    // using cel shading for cat!
    //specular = floor(specular * 4 + 4) / 8.7;
  }
	
	// total reflective profile
	color = vec4(MatAmb + specular*MatSpec + diffuse*MatDif, 1.0) * texColor;
//   if (flip > 0)
//   	color = floor(color * 4 + 4) / 8.7; // using cel shading for cat!
}
*/