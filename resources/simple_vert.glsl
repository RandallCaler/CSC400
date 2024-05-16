#version  330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 LS;

uniform vec3 lightDir;

out OUT_struct {
	vec3 fPos;
	vec3 fragNor;
	vec2 vTexCoord;
	vec4 fPosLS;
	vec3 vColor;
    vec3 lightDir;
    vec3 EPos;
} out_struct;

void main() {

  /* First model transforms */
  gl_Position = P * V * M * vec4(vertPos.xyz, 1.0);

	/* the position in world coordinates */
  out_struct.fPos = (M*vec4(vertPos, 1.0)).xyz;
	/* the normal */
  out_struct.fragNor = (M*vec4(vertNor, 0.0)).xyz;
  /* pass through the texture coordinates to be interpolated */
  out_struct.vTexCoord = vertTex;
  /* The vertex in light space TODO: fill in appropriately */
  out_struct.fPosLS = LS*M*vec4(vertPos.xyz, 1.0);
  /* a color that could be blended - or be shading */
  out_struct.vColor = vec3(max(dot(out_struct.fragNor, normalize(lightDir)), 0));
  out_struct.lightDir = lightDir;
  out_struct.EPos = vec3(V*M*vec4(vertPos, 1.0));
}



















/*#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightDir; // position of light source in model space

out vec3 fragNor;
out vec3 LDir;
out vec3 EPos;

void main() {
	// translate model to view space
    vec4 viewPos = V * M * vertPos;

    // vertex normal in view space
    fragNor = (transpose(inverse(V * M)) * vec4(vertNor, 0.0)).xyz;

    // direction of light source in view space
    LDir = mat3(V) * lightDir;
	
	// vertex position in view space
	EPos = vec3(viewPos);

    // complete vertex shading
    gl_Position = P * viewPos;
}*/
 