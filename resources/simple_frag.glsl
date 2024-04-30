#version 330 core 

out vec4 color;

// reflective properties of material
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float MatShine;

in vec3 fragNor; // vertex normal in view space
in vec3 LDir; // light source vector in view space
in vec3 EPos; // vertex position in view space

void main() {
	// normal unit vector to vertex geometry
	vec3 normal = normalize(fragNor);
	// unit vector toward light source
	vec3 light = normalize(LDir);
	// unit vector angularly halfway between light and camera
	vec3 halfway = normalize(light - normalize(EPos));
	
	// diffusion coefficient
	float diffuse = max(0, dot(normal, light));
	diffuse = floor(diffuse * 5) / 5.0;
	// specular coefficient
	float specular = pow(max(0, dot(normal, halfway)), MatShine);
	specular = floor(specular * 5) / 5.0;
	
	// total reflective profile
	color = vec4(MatAmb + vec3(specular)*MatSpec, 1.0) + vec4(vec3(diffuse)*MatDif, 1.0);
}
