#version 330 core 

out vec4 color;
in vec3 regionColor;
in float h_vert;

void main() {

	color = vec4(regionColor, 1.0);
}
