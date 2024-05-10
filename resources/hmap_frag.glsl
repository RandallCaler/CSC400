#version 330 core 

out vec4 color;
in float regionColor;
in float h_vert;

void main() {
	vec3 regionColorVec;
	
	regionColorVec = 
		(regionColor == 1.0) ? vec3(1, 0, 0) : 
		(regionColor == 2.0) ? vec3(0, 1, 0) : 
		(regionColor == 3.0) ? vec3(0, 0, 1) : vec3(0, 0, 0);

	color = vec4(vec3(regionColorVec) * h_vert, 1);
}

