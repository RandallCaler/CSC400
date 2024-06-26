#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertPos;

// Values that stay constant for the whole mesh.
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main(){

  gl_Position = P * V * M * vec4(vertPos.xyz, 1.0);

}