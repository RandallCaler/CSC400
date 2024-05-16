#version  330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec3 region;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out float h_vert;
out vec3 regionColor;
out vec3 fragNor;

void main() {
    regionColor = region;
    fragNor = vertNor;
    h_vert = vertPos.y;

    // complete vertex shading
    gl_Position = P * V * M * vec4(vertPos, 1);
}
 