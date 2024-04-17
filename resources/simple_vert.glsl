#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPos;

out vec3 fragNor;
out vec3 lightDir;
out vec3 EPos;

void main()
{
	gl_Position = P * V * M * vertPos;
	fragNor = (V*M * vec4(vertNor, 0.0)).xyz;
	lightDir = vec3(V*(vec4(lightPos - (M*vertPos).xyz, 0.0)));
	EPos = vec3(1); //PULLED for release
}
