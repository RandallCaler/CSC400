#version 330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec2 vertTex;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec2 vTexCoord;

void main() {
	// translate model to view space
    vec4 pos = P * V * M * vertPos;

    // pass through texture
    vTexCoord = vertTex;

    // complete vertex shading
    gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
}