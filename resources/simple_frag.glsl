
#version 330 core
uniform sampler2D shadowDepth;

out vec4 Outcolor;

/*uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float MatShine;*/

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
   vec3 lightDir;
   vec3 EPos;
} in_struct;

/* returns 1 if shadowed */
/* called with the point projected into the light's coordinate space */


//using Trowbridge-Reitz Normal Distribution Function
float D (float alpha, vec3 N, vec3 H) {

	float dot_prod = max(0, dot(N, H));
	float num = pow(alpha, 2.0);
	float denom = max(0.0005, pow(pow(dot_prod, 2.0) * (pow(alpha, 2.0) - 1.0) + 1.0, 2.0));
	return num / denom;

}

float G1 (float alpha, vec3 N, vec3 X) {
	float num = max(0, dot(N, X));
	float k = alpha/2.0;
	float denom = max(0.0005, num * (1.0 - k) + k);
	return num / denom;
}

float G (float alpha, vec3 N, vec3 V, vec3 L) {
	return G1 (alpha, N, V) * G1 (alpha, N, L);
}


//Fresnel Schlick function for specular
vec3 F (vec3 F0, vec3 V, vec3 N) {
	return F0 + (vec3(1.0) - F0) * pow(1 - max(0, dot(V, N)), 5.0);
}



float TestShadow(vec4 LSfPos) {

  //0.005 * tan (acos(nDotl)) is better/more precise
  float depth_buffer = 0.1;

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

void main() {

	float Shade;
	float amb = 0.5;

  	vec3 N = normalize(in_struct.fragNor);
	
	//  temporarily set light dir to constant vector
	vec3 L = normalize(in_struct.lightDir);
	//vec3 L = normalize(vec3(.5, 1.0, -0.5));
	vec3 V = normalize(in_struct.EPos * -1);
	vec3 H = normalize(L + V);
	
	vec3 Ks = F(reflectance, V, N);
	vec3 Kd = vec3(1.0) - Ks;

	vec3 lambert = albedo;
	vec3 num = D (roughness, N, H) * G (roughness, N, V, L) * F (reflectance, V, N); //using cook torrance
	float denom = max(0, dot(V, N)) * max(0, dot(L, N));
	vec3 BRDF = Kd * lambert + (num / denom);
	//color = vec4(BRDF * lightColor, 1.0);


	float x_val = max(amb*emissivity.x, emissivity.x + BRDF.x * lightColor.x * max(0.005, dot(N, L)));
	float y_val = max(amb*emissivity.y, emissivity.y + BRDF.y * lightColor.y * max(0.005, dot(N, L))); 
	float z_val = max(amb*emissivity.z, emissivity.z + BRDF.z * lightColor.z * max(0.005, dot(N, L))); 
	vec4 baseColor = vec4(x_val, y_val, z_val, 1.0);


  	//vec4 BaseColor = vec4(in_struct.vColor, 1);
  	//vec4 texColor0 = vec4(1.0, 0, 0, 1.0);

	Shade = TestShadow(in_struct.fPosLS);

	Outcolor = amb*(baseColor) + (1.0-Shade)*baseColor;
	
}



/*#version 330 core 

out vec4 color;

// reflective properties of material
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float MatShine;

in vec3 fragNor; // vertex normal in view space
in vec3 LDir; // light source vector in view space
in vec3 EPos; // vertex position in view space

void main() {
	// normal unit vector to vertex geometry
	vec3 normal = normalize(fragNor);
	// unit vector toward light source
	vec3 light = normalize(LDir);
	// unit vector angularly halfway between light and camera
	vec3 halfway = normalize(light - normalize(EPos));
	
	// diffusion coefficient
	float diffuse = max(0, dot(normal, light));
	diffuse = floor(diffuse * 5) / 5.0;
	// specular coefficient
	float specular = pow(max(0, dot(normal, halfway)), MatShine);
	specular = floor(specular * 5) / 5.0;
	
	// total reflective profile
	color = vec4(MatAmb + vec3(specular)*MatSpec, 1.0) + vec4(vec3(diffuse)*MatDif, 1.0);
}
*/