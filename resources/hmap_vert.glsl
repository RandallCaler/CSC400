#version  330 core
layout(location = 0) in vec3 vertPos;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out float h_vert;

void main() {
    h_vert = vertPos.y;

    // complete vertex shading
    gl_Position = P * V * M * vec4(vertPos, 1);
}
 