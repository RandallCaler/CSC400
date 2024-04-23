#version  330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
layout(location = 3) in ivec4 boneIds; 
layout(location = 4) in vec4 weights;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
uniform vec3 lightPos;

const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec3 fragNor;
out vec3 lightDir;
out vec3 EPos;
out vec2 vTexCoord;

void main() {

  vec4 totalPosition = vec4(0.0f);

  for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >=MAX_BONES) 
        {
            totalPosition = vec4(vertPos,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(vertPos,1.0f);
        totalPosition += localPosition * weights[i];
        vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * vertNor;
   }
  
  gl_Position = P * V *M * totalPosition;
  //gl_Position = P * V *M * vec4(vertPos.xyz, 1.0);
  vec3 wPos = vec3(M * vec4(vertPos.xyz, 1.0));
  fragNor = (V * M * vec4(vertNor, 0.0)).xyz; 
  lightDir = (V*(vec4(lightPos - wPos, 0.0))).xyz;
  EPos = (V*M*vec4(wPos, 1.0)).xyz;
  
  /* pass through the texture coordinates to be interpolated */
  vTexCoord = vertTex;

}
