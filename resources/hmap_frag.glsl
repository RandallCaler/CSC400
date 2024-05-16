#version 330 core 

uniform sampler2D shadowDepth;


out vec4 color;
in float h_vert;

in OUT_struct {
   vec3 fPos;
   vec3 fragNor;
   vec4 fPosLS;
   vec3 vColor;
} in_struct;


float TestShadow(vec4 LSfPos) {

  //0.005 * tan (acos(nDotl)) is better/more precise
  float depth_buffer = 0.009;

	//1: shift the coordinates from -1, 1 to 0, 1
  vec3 fLS = (vec3(LSfPos) + vec3(1.0)) * 0.5;

	//2: read off the stored depth (.) from the ShadowDepth, using the shifted.xy 
  float depth = texture(shadowDepth, fLS.xy).r;

	//3: compare to the current depth (.z) of the projected depth
  if (fLS.z > depth + depth_buffer)
    return 1.0;
  return 0.0;

	//4: return 1 if the point is shadowed

}

void main() {

  float Shade;
  float amb = 0.3;

  vec4 BaseColor = vec4(in_struct.vColor, 1);

  Shade = TestShadow(in_struct.fPosLS);

  color = amb*(vec4(vec3(h_vert), 1)) + (1.0-Shade)*vec4(vec3(h_vert), 1);
}
