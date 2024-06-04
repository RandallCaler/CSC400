#version 330 core 

uniform sampler2D shadowDepth;


out vec4 color;
in vec3 regionColor;
in vec3 fragNor;
in float h_vert;

in OUT_struct {
   vec3 fPos;
   vec3 fragNor;
   vec4 fPosLS;
   vec3 vColor;
   vec3 lightDir;
} in_struct;



uniform float offset[10] = float[]( 0.025, 0.05, 0.075, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4 );
uniform float weight[10] = float[]( 0.2270270270, 0.1945945946, 0.1216216216,
0.0540540541, 0.0162162162, 0.0059383423, 0.00129128391, 0.00129128391, 0.00129128391, 0.00129128391);


float TestShadow(vec4 LSfPos) {

  //0.005 * tan (acos(nDotl)) is better/more precise
  float depth_buffer = 0.0009;

  if (LSfPos.x > 1 || LSfPos.x < -1 || LSfPos.y > 1 || LSfPos.y < -1) {
    return 0.0;
  }

	//1: shift the coordinates from -1, 1 to 0, 1
  vec3 fLS = (vec3(LSfPos) + vec3(1.0)) * 0.5;

  float count = 0;
  float in_shadow;

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      in_shadow = texture(shadowDepth, (fLS.xy + (vec2(offset[i], offset[j])/16384.0))).r;
      if (fLS.z > in_shadow + depth_buffer)
        count += 1;
    }
  }
  
  return (count)/500.0;


	//2: read off the stored depth (.) from the ShadowDepth, using the shifted.xy 
  

	//3: compare to the current depth (.z) of the projected depth
  

	//4: return 1 if the point is shadowed

}


void main() {

  float Shade;
  float amb = 0.3;

  vec4 BaseColor = vec4(in_struct.vColor, 1);

  Shade = TestShadow(in_struct.fPosLS);

  // color = amb*(vec4(vec3(h_vert), 1)) + (1.0-Shade)*vec4(vec3(h_vert), 1);
  //vec3 testLightDir = normalize(vec3(1, -1, 0.5));
  
  float intensity = max(dot(in_struct.lightDir, normalize(in_struct.fragNor)), 0);
  
  //amb*(baseColor) + (1.0-Shade)*baseColor
  color = amb*(BaseColor) + (1.0-Shade) * BaseColor;
  //color = vec4(vec3(regionColor) * intensity, 1.0);
  //color = BaseColor;
}

