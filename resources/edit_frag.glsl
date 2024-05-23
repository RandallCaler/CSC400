#version 330 core

// Output data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform vec3 PickingColor;

void main(){

    color = vec4(PickingColor, 1);

}
