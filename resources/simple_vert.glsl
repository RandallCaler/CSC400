#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightDir; // position of light source in model space

out vec3 fragNor;
out vec3 LDir;
out vec3 EPos;

void main() {
	// translate model to view space
    vec4 viewPos = V * M * vertPos;

    // vertex normal in view space
    fragNor = (transpose(inverse(V * M)) * vec4(vertNor, 0.0)).xyz;

    // direction of light source in view space
    LDir = mat3(V) * lightDir;
	
	// vertex position in view space
	EPos = vec3(viewPos);

    // complete vertex shading
    gl_Position = P * viewPos;
}
 