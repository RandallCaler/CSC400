#version 330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec2 vTexCoord;

void main() {
    vec4 pos = P * V * M * vertPos;
    // pass through texture
    vTexCoord = vertTex;

    // complete vertex shading
    gl_Position = pos;
}