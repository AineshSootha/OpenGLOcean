#version 430
#define M_PI 3.14159265358979323846264338327950288


layout(local_size_x = 16, local_size_y = 16) in;
// layout(rgba32f, binding = 0) writeonly uniform image2D img_output;
layout(rgba32f, binding = 1) uniform image2D inOutTex0;
layout(rgba32f, binding = 2) uniform image2D inOutTex1;
layout(std430, binding = 0) buffer reverse_indices{
    int revInd[256];
};

uniform int N;
uniform int currStage;
uniform int inOutDecide;

vec2 multiply(vec2 h0, vec2 wkt){
  vec2 res = vec2(0.0, 0.0);
  res.x = h0.x * wkt.x - h0.y * wkt.y;
  res.y = h0.x * wkt.y + h0.y * wkt.x;
  return res;
}

vec2 twiddleCompute(int x){
    float twiddleExponent = mod(x * float(N)/ pow(2, currStage + 1), N);
    twiddleExponent = 2.0 * M_PI * twiddleExponent / float(N);
    return vec2(cos(twiddleExp), sin(twiddleExp));
}

void main(){
    ivec2 pos = ivec2( gl_GlobalInvocationID.xy );
    vec4 inputVec1;
    vec4 inputVec2;
    int span = pow(2, currStage);
    vec2 twiddle = twiddleCompute(pos.x);
    // vec4 butterflyIndices_T = imageLoad(butterfly, ivec2(stage, pos.x));
    ivec2 firstIdx;  
    ivec2 secondIdx;
    int topOrBottom = pos.y & pow(2, currStage + 1) - 1 ;
    if(currStage == 0){
        if(topOrBottom < pow(2, currStage)){
            firstIdx = revInd[pos.x];
            secondIdx = revInd[pos.x + 1];
        }else{
            firstIdx = revInd[pos.x - 1];
            secondIdx = revInd[pos.x];
        }
    }else{
        if(topOrBottom < pow(2, currStage)){
            firstIdx = revInd[pos.x];
            secondIdx = revInd[pos.x  + pow(2, currStage)];
        }
        else{
            firstIdx = revInd[pos.x - pow(2, currStage)];
            secondIdx = revInd[pos.x];
        }   
    }
    if(inOutDecide == 0){
        inputVec1 = imageLoad(inOutTex0, firstIdx);
        inputVec2 = imageLoad(inOutTex0, secondIdx);
    }else{
        inputVec1 = imageLoad(inOutTex1, firstIdx);
        inputVec2 = imageLoad(inOutTex1, secondIdx);
    }
    vec2 res = multiply(twiddle, inputVec2) + inputVec1;

    if(inOutDecide == 0){
        imageStore(inOutTex1, pos, vec4(res, 0.0, 1.0));
    }else{
        imageStore(inOutTex0, pos, vec4(res, 0.0, 1.0));
    }




}