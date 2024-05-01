#version 330 core 

out vec4 color;
in float h_vert;

void main() {
	color = vec4(vec3(h_vert), 1);
}
