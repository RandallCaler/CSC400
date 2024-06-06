#version 330 core

in vec2 texCoord;
out vec4 color;
uniform sampler2D texBuf;

void main(){

	float depth =texture( texBuf, texCoord ).r;
	//TODO should depth be modified to make the visual debugging more useful?
	color = vec4(vec3(depth), 1.0);

}
