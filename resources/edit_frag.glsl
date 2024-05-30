#version 330 core

// Output data
out vec4 color;
uniform sampler2D Texture0;

// Values that stay constant for the whole mesh.
uniform vec3 PickingColor;

void main(){

    color = vec4(PickingColor, 1);

}
