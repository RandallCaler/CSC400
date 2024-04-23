#version 330 core
uniform sampler2D texture_diffuse1;
uniform int flip;

in vec2 vTexCoord;
in vec3 fragNor;
in vec3 lightDir;
out vec4 Outcolor;

void main() {
    vec4 texColor0 = texture(texture_diffuse1, vTexCoord);
    vec3 normal = normalize(fragNor);
    vec3 light = normalize(lightDir);
    if(flip == 0){
        normal = -normal;
    }
    float dC = max(0, dot(normal, light));
    Outcolor = dC * texColor0;

}
