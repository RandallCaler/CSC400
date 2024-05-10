#version  330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in float region;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out float h_vert;
out float regionColor;

void main() {
    regionColor = region;
    h_vert = (vertPos.y + 1)/2;

    // complete vertex shading
    gl_Position = P * V * M * vec4(vertPos, 1);
}
 