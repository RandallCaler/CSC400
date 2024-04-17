#version 330 core 

out vec4 color;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float MatShine;

//interpolated normal and light vector in camera space
in vec3 fragNor;
in vec3 lightDir;
//position of the vertex in camera space
in vec3 EPos;

void main()
{
	vec3 V = normalize(EPos * -1);
  	vec3 real_light = EPos + vec3(0, 1, 2);
	vec3 normal = normalize(fragNor);
	vec3 light = normalize(real_light);
	float dC = max(0, dot(normal, light));
	dC = floor(dC * 5) / 5.0;
	
	
	color = vec4(MatAmb + dC*MatDif, 1.0);
}
