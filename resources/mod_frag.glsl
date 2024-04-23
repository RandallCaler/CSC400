#version 330 core

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

uniform int flip;

in vec2 vTexCoord;

out vec4 Outcolor;

//interpolated normal and light vector in camera space
in vec3 fragNor;
in vec3 lightDir;
//position of the vertex in camera space
in vec3 EPos;

void main() {
  vec4 diffuse = texture(texture_diffuse1, vTexCoord);
  vec4 specular = texture(texture_specular1, vTexCoord);
  vec4 normalMap = texture(texture_normal1, vTexCoord);
  vec4 ambient = texture(texture_height1, vTexCoord);

  vec3 normal = normalize(fragNor);
  if(flip == 0){
        normal = -normal;
    }

  vec3 light = normalize(lightDir);
  float dC = max(0, dot(normal, light));

  vec3 H = normalize(normalize(-EPos)+light);
  Outcolor = 0.7 * (ambient + vec4(dC*diffuse.xyz, 1.0) + vec4(specular.xyz * pow(max(dot(H, normal), 0), 30.0), 1.0));


}
