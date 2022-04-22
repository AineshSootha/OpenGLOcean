#version 430
#define M_PI 3.14159265358979323846264338327950288


layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba32f, binding = 0) writeonly uniform image2D img_output;
layout(rgba32f, binding = 1) readonly uniform image2D img_h0;
layout(rgba32f, binding = 2) readonly uniform image2D img_h0MinusK;

uniform float T;
uniform int N;
uniform int startL;
uniform float g = 9.81;



vec2 k_(ivec2 pos){
  vec2 k = vec2((2*M_PI*(pos.x - float(N) / 2))/startL, (2*M_PI*(pos.y - float(N) / 2))/startL); 
  return k;
}


vec2 multiply(vec2 h0, vec2 wkt){
  vec2 res = vec2(0.0, 0.0);
  res.x = h0.x * wkt.x - h0.y * wkt.y;
  res.y = h0.x * wkt.y + h0.y * wkt.x;
  return res;
}

vec2 conjugate(vec2 complexNum){
  return vec2(complexNum.x, -complexNum.y);
}

void main() {
  // base pixel colour for image
  ivec2 pos = ivec2( gl_GlobalInvocationID.xy ); 
  vec2 k = k_(pos);
  float magK = length(k) < 0.0001 ? 0.0001 : length(k);
  float w = sqrt(magK * g);
  float realW = cos(w*T);
  float imagW = sin(w*T);
  vec2 h0k = imageLoad(img_h0, pos).rg;
  vec2 prelimRes1 = multiply(h0k, vec2(realW, imagW));
  vec2 h0MinusK = imageLoad(img_h0MinusK, pos).rg;
  h0MinusK = conjugate(h0MinusK);
  vec2 prelimRes2 = multiply(h0MinusK, vec2(realW, -imagW));
  
  vec2 finalRes = prelimRes1 + prelimRes2;
  vec2 ux = vec2(0.0, -k.x / magK);
  vec2 uy = vec2(0.0, -k.y / magK);
  vec4 res = vec4(multiply(ux, finalRes), multiply(uy, finalRes));


  imageStore(img_output, pos, vec4(res));  
}
