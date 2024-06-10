#version 330 core 

uniform sampler2D shadowDepth;
uniform sampler2D terrain0;
uniform sampler2D terrain1;
uniform sampler2D terrain2;
uniform sampler2D terrain3;
uniform sampler2D terrain4;
uniform sampler2D terrain5;
uniform sampler2D terrain6;
uniform float h_min;
uniform float h_max;
uniform float fTime;

out vec4 color;
in vec3 fRegion;
in vec3 fragNor;
in vec2 normDistortion;

in OUT_struct {
  vec3 fPos;
  vec3 ePos;
  vec3 fragNor;
  vec4 fPosLS;
  vec3 vColor;
  vec3 lightDir;
  vec4 clipSpace;
} in_struct;

uniform float offset[10] = float[]( 0.025, 0.05, 0.075, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4 );
uniform float weight[10] = float[]( 0.2270270270, 0.1945945946, 0.1216216216,
0.0540540541, 0.0162162162, 0.0059383423, 0.00129128391, 0.00129128391, 0.00129128391, 0.00129128391);

vec3 getTerrainTexture() {
  float slopeFactor = length(vec2(in_struct.fragNor.x, in_struct.fragNor.z));
  vec2 texCoord = in_struct.fPos.xz/2;
  vec3 totalTex = vec3(0);
  if (fRegion.x > 0) {
    totalTex += fRegion.x * texture(terrain0, texCoord).xyz * slopeFactor;
    totalTex += fRegion.x * texture(terrain1, texCoord).xyz * (1-slopeFactor);
  }
  if (fRegion.y > 0) {
    totalTex += fRegion.y * texture(terrain2, texCoord).xyz * slopeFactor;
    totalTex += fRegion.y * texture(terrain3, texCoord).xyz * (1-slopeFactor);
  }
  if (fRegion.z > 0) {
    totalTex += fRegion.z * texture(terrain4, texCoord).xyz * slopeFactor;
    totalTex += fRegion.z * texture(terrain5, texCoord).xyz * (1-slopeFactor);
  }

  if (totalTex == vec3(0)) {
    if (in_struct.fPos.y < h_min + 0.1) {
      totalTex = texture(terrain6, in_struct.fPos.xz/10 + vec2(fTime/25, 0)).xyz;
    }
    else {
      totalTex = texture(terrain0, texCoord).xyz;
    }
  }

  return totalTex;
}

float TestShadow(vec4 LSfPos) {

  //0.005 * tan (acos(nDotl)) is better/more precise
  float depth_buffer = 0.0009;

  if (LSfPos.x > 1 || LSfPos.x < -1 || LSfPos.y > 1 || LSfPos.y < -1) {
    return 0.0;
  }

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

  return (count)/100.0;


	//2: read off the stored depth (.) from the ShadowDepth, using the shifted.xy 
  

	//3: compare to the current depth (.z) of the projected depth
  

	//4: return 1 if the point is shadowed

}


void main() {

  float Shade;
  float amb = 0.3;

  vec3 terrainTex = getTerrainTexture();

  Shade = TestShadow(in_struct.fPosLS);

  // normal unit vector to vertex geometry
	vec3 normal = normalize(in_struct.fragNor + vec3(normDistortion.x, 0.0, normDistortion.y));
	// unit vector toward light source
	vec3 light = normalize(in_struct.lightDir);
	// unit vector angularly halfway between light and camera
	vec3 halfway = normalize(light - normalize(in_struct.ePos));

	// diffusion coefficient
	float diffuse = max(0, dot(normal, light));
	// specular coefficient
	float specular = pow(max(0, dot(normal, halfway)), 10);
	
	color = vec4(vec3(0.1) + vec3((specular + diffuse) * (1 - Shade)) * terrainTex, 1.0);
}

