#version 330
layout(location = 0) in vec4 vertPos;
uniform mat4 P;
uniform mat4 V;

void main()
{
	gl_Position = P * V * vertPos;
}
