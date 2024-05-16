#version  330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 LS;

uniform vec3 lightDir;

out OUT_struct {
	vec3 fPos;
	vec3 fragNor;
	vec4 fPosLS;
	vec3 vColor;
} out_struct;

out float h_vert;

void main() {
    h_vert = mix(0, 1, (vertPos.y + 1)/2);

    // complete vertex shading
    gl_Position = P * V * M * vec4(vertPos, 1);
    

	/* the position in world coordinates */
    out_struct.fPos = (M*vec4(vertPos, 1.0)).xyz;
        /* the normal */
    out_struct.fragNor = (M*vec4(vertNor, 0.0)).xyz;
    /* The vertex in light space TODO: fill in appropriately */
    out_struct.fPosLS = LS*M*vec4(vertPos.xyz, 1.0);
    /* a color that could be blended - or be shading */
    out_struct.vColor = vec3(max(dot(out_struct.fragNor, normalize(lightDir)), 0));

}
 