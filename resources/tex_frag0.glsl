#version 330 core

uniform sampler2D Texture0;
uniform float MatShine;
uniform vec3 MatAmb;
uniform vec3 MatSpec;
uniform vec3 MatDif;

uniform int flip;

in vec2 vTexCoord;

out vec4 Outcolor;

//interpolated normal and light vector in camera space
in vec3 fragNor;
in vec3 lightDir;
//position of the vertex in camera space
in vec3 EPos;


void main() {
  vec4 texColor0 = texture(Texture0, vTexCoord);

  vec3 real_light = EPos + vec3(0, 1, 2);

  vec3 normal = normalize(fragNor);
  if (flip < 1)
  	normal *= -1.0f;
  vec3 light = normalize(real_light);
  float dC = max(0, dot(normal, light));

  vec3 V = normalize(EPos * -1);

	
	vec3 H = normalize(light + V);
	float sL = max(0, dot(normal, H));

	float spec = pow(sL, MatShine);
  if (flip > 0)
	  dC = floor(dC * 4 + 4) / 8.7;    // using cel shading for cat!

  Outcolor = vec4(MatAmb + dC*texColor0.xyz + MatSpec * spec, 1.0);

  //to confirm texture coordinates
  //Outcolor = vec4(vTexCoord.x, vTexCoord.y, 0, 0);
}

