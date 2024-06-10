#version 330 core
uniform sampler2D Texture0;
uniform sampler2D shadowDepth;

out vec4 Outcolor;


in OUT_struct {
   vec3 fPos;
   vec3 fragNor;
   vec2 vTexCoord;
   vec4 fPosLS;
   vec3 vColor;
} in_struct;


uniform float offset[10] = float[]( 0.025, 0.05, 0.075, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4 );
uniform float weight[10] = float[]( 0.2270270270, 0.1945945946, 0.1216216216,
0.0540540541, 0.0162162162, 0.0059383423, 0.00129128391, 0.00129128391, 0.00129128391, 0.00129128391);

/* returns 1 if shadowed */
/* called with the point projected into the light's coordinate space */
float TestShadow(vec4 LSfPos) {

   //0.005 * tan (acos(nDotl)) is better/more precise
  float depth_buffer = 0.0009;

	//1: shift the coordinates from -1, 1 to 0, 1
  vec3 fLS = (vec3(LSfPos) + vec3(1.0)) * 0.5;

  float count = 0;
  float in_shadow;

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      in_shadow = texture(shadowDepth, (fLS.xy + (vec2(offset[i], offset[j])/256.0))).r;
      if (fLS.z > in_shadow + depth_buffer)
        count += 1;
    }
  }
  
  return (count)/500.0;

}

void main() {

  float Shade;
  float amb = 0.3;

  vec4 texColor0 = texture(Texture0, in_struct.vTexCoord);

  Shade = TestShadow(in_struct.fPosLS);

  Outcolor = amb*texColor0*vec4(1.0) + 0.7*(1.0-Shade)*texColor0*vec4(1.0);

}
