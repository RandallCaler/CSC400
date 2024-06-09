#version 330 core

uniform sampler2D alphaTexture;

in vec4 partCol;

out vec4 outColor;


void main()
{
	float alpha = texture(alphaTexture, gl_PointCoord).r;

	outColor = vec4(partCol.xyz, alpha);
}
