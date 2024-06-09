
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



uniform float offset[10] = float[]( 0.025, 0.05, 0.075, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4 );
uniform float weight[10] = float[]( 0.2270270270, 0.1945945946, 0.1216216216,
0.0540540541, 0.0162162162, 0.0059383423, 0.00129128391, 0.00129128391, 0.00129128391, 0.00129128391);

/* returns 1 if shadowed */
/* called with the point projected into the light's coordinate space */


//using Trowbridge-Reitz Normal Distribution Function
float D (float alpha, vec3 N, vec3 H) {

	float dot_prod = max(0, dot(N, H));
	float num = pow(alpha, 2.0);
	float denom = max(0.0005, 3.1415 * pow(pow(dot_prod, 2.0) * (pow(alpha, 2.0) - 1.0) + 1.0, 2.0));
	return num / denom;

}

float G1 (float alpha, vec3 N, vec3 X) {
	float num = max(0, dot(N, X));
	float k = alpha/2.0;
	float denom = max(0.0005, 4 * num * (1.0 - k) + k);
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
  float depth_buffer = 0.0009;

	//1: shift the coordinates from -1, 1 to 0, 1
  vec3 fLS = (vec3(LSfPos) + vec3(1.0)) * 0.5;

  float count = 0;
  float in_shadow;

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      in_shadow = texture(shadowDepth, (fLS.xy + (vec2(offset[i], offset[j])/256.0))).r;
      if (fLS.z > in_shadow + depth_buffer)
        count += 1;
    }
  }
  
  return (count)/500.0;

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
	

	Shade = TestShadow(in_struct.fPosLS);

	Outcolor = 0.7*(1.0-Shade)*vec4(albedo, 1.0); 
	
}
