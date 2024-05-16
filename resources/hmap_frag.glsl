#version 330 core 

out vec4 color;
in vec3 regionColor;
in vec3 fragNor;
in float h_vert;


void main() {
	vec3 testLightDir = normalize(vec3(1, -1, 0.5));
	float intensity = max(-dot(testLightDir, fragNor), 0);
	color = vec4(vec3(intensity) * regionColor, 1.0);
}

