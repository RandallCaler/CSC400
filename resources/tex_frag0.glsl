#version 330 core 

out vec4 color;

// reflective properties of material
uniform sampler2D Texture0;
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float MatShine;
uniform int flip;

in vec2 vTexCoord; // texture data
in vec3 fragNor; // vertex normal in view space
in vec3 LDir; // light source vector in view space
in vec3 EPos; // vertex position in view space

void main() {
	// texture color at this location
	vec4 texColor = texture(Texture0, vTexCoord);
	// normal unit vector to vertex geometry
	vec3 normal = normalize(fragNor);
	// unit vector toward light source
	vec3 light = normalize(LDir);
	// unit vector angularly halfway between light and camera
	vec3 halfway = normalize(light - normalize(EPos));
	
	// diffusion coefficient
	float diffuse = max(0, dot(normal, light));
	// specular coefficient
	float specular = pow(max(0, dot(normal, halfway)), MatShine);
  if (flip > 0) {
    //diffuse = floor(diffuse * 4 + 4) / 8.7;    // using cel shading for cat!
    //specular = floor(specular * 4 + 4) / 8.7;
  }
	
	// total reflective profile
	color = (vec4(MatAmb + vec3(specular)*MatSpec, 1.0) + vec4(vec3(diffuse)*MatDif, 1.0)) * texColor;
  if (flip > 0)
  	color = floor(color * 4 + 4) / 8.7; // using cel shading for cat!
}
