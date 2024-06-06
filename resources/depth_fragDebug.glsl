#version 330 core

/* write out depth to show transforms working */
out vec4 Outcolor;

void main() {
	Outcolor = vec4(0, gl_FragCoord.z, gl_FragCoord.z, 1.0);
}

