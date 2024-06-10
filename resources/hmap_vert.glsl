#version  330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in float region;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 LS;
uniform float fTime;
uniform sampler2D DuDvMap;

uniform vec3 lightDir;

out OUT_struct {
    vec3 fPos;
    vec3 ePos;
    vec3 fragNor;
    vec4 fPosLS;
    vec3 vColor;
    vec3 lightDir;
    vec4 clipSpace;
} out_struct;

out vec3 fRegion;
out vec3 fragNor;
out vec2 normDistortion;

void main() {
    float mirageAmp;
    int bands = 4096;
    if (region < 0.01) {
        fRegion = vec3(0, 0, 0);
        mirageAmp = 0.15;
    }
    else if (region < 1.001) {
        fRegion = vec3(1, 0, 0);
        mirageAmp = 0.75;
    }
    else if (region < 2.001) {
        fRegion = vec3(0, 1, 0);
        mirageAmp = 0.25;
    }
    else if (region < 3.001) {
        fRegion = vec3(0, 0, 1);
        mirageAmp = 0.25;
    }
    else {
        fRegion = vec3(1);
        mirageAmp = 0;
    }

    fragNor = vertNor;

    // complete vertex shading
    vec3 vPos = vec3(V * M * vec4(vertPos, 1));
    out_struct.ePos = vPos;
    vec3 flatViewVec = normalize(vec3(vPos.x, 0.0, vPos.z));
    float lPos = dot(vPos, flatViewVec);
    out_struct.clipSpace = P * V * M * vec4(vertPos, 1);
    // float centerPosX = round(gl_Position.x / gl_Position.w * bands) / bands * gl_Position.w;
    float centerPosX = gl_Position.x;
    if (lPos > 100) {
        float mixValue = (vertPos.y + 37.5)/75;
	    // gl_Position.x = centerPosX + max(1*(1 - mixValue * mixValue) - 0.25, 0) * (centerPosX - gl_Position.x) * (lPos - 43)/64 * sin((0.5 - mixValue) * 10 * fTime);
	    gl_Position.x = centerPosX + 2.5*mirageAmp*(1 - mixValue * mixValue) * (lPos - 95)/200 * sin((1 - mixValue) * 10 * fTime);
    }
    

	/* the position in world coordinates */
    out_struct.fPos = (M*vec4(vertPos, 1.0)).xyz;
    normDistortion = texture(DuDvMap, out_struct.fPos.xz + vec2(-fTime/200, fTime/200)).rg * 2 - 1;
        /* the normal */
    out_struct.fragNor = normalize((M*vec4(vertNor, 0.0)).xyz);
    /* The vertex in light space TODO: fill in appropriately */
    out_struct.fPosLS = LS * M * vec4(vertPos.xyz, 1.0);
    /* a color that could be blended - or be shading */
    out_struct.vColor = vec3(max(dot(out_struct.fragNor, normalize(lightDir)), 0));
    out_struct.lightDir = normalize(lightDir);

    gl_Position = out_struct.clipSpace, 1.0;
}
 