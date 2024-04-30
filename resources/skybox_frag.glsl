#version 330 core 

out vec4 color;

// reflective properties of material
uniform sampler2D Texture0;

in vec2 vTexCoord; // texture data

void main() {
	// texture color at this location
	color = texture(Texture0, vTexCoord);
}
