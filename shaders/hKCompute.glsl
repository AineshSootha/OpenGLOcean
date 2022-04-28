#version 430
#define M_PI 3.14159265358979323846264338327950288


layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba32f, binding = 0) writeonly uniform image2D hkz;
layout(rgba32f, binding = 3) writeonly uniform image2D hkx;
layout(rgba32f, binding = 4) writeonly uniform image2D hky;

layout(rgba32f, binding = 1) readonly uniform image2D img_h0;
layout(rgba32f, binding = 2) readonly uniform image2D img_h0MinusK;


uniform float T;
uniform int N;
uniform int startL;
uniform float g = 9.81;

int alias(int x)
{
    if (x > N / 2)
        x -= N;
    return x;
}


vec2 k_(vec2 pos){
  vec2 k = vec2((2*M_PI*(pos.x))/startL, (2*M_PI*(pos.y))/startL); 
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
  ivec2 pos_ = ivec2(gl_GlobalInvocationID.xy);
  float n = pos_.x > float(N) / 2 ? pos_.x - float(N) :  pos_.x;
  float m = pos_.y > float(N) / 2 ? pos_.y - float(N) :  pos_.y;
  vec2 pos = vec2(n,m);
  vec2 k = k_(pos);
  float magK = length(k) < 0.0001 ? 0.0001 : length(k);
  float w = sqrt(magK * g);
  float realW = cos(w*T);
  float imagW = sin(w*T);
  vec2 h0k = imageLoad(img_h0, pos_).rg;
  vec2 prelimRes1 = multiply(h0k, vec2(realW, imagW));
  vec2 h0MinusK = imageLoad(img_h0MinusK, pos_).rg;
  h0MinusK = conjugate(h0MinusK);
  vec2 prelimRes2 = multiply(h0MinusK, vec2(realW, -imagW));
  
  vec2 finalRes = prelimRes1 + prelimRes2;
  vec2 ux = vec2(0.0, -k.x / magK);
  vec2 uy = vec2(0.0, -k.y / magK);
  vec2 resZ = vec2(clamp(finalRes.x, -100, 100), clamp(finalRes.y, -100, 100));
  vec2 resY = vec2(multiply(resZ, uy));
  vec2 resX = vec2(multiply(resZ, ux));

  if(k.x == 0.0 && k.y == 0.0){
    resZ = vec2(0.0);
    resX = vec2(0.0);
    resY = vec2(0.0);
  }
  
  imageStore(hkz, pos_, vec4(resZ, resZ));  
  imageStore(hkx, pos_, vec4(resX, resX));  
  imageStore(hky, pos_, vec4(resY, resY));
}
