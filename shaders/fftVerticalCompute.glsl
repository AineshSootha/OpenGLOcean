#version 430
#define M_PI 3.14159265358979323846264338327950288


layout(local_size_x = 16, local_size_y =16) in;
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

vec2 twiddleCompute(int y){
    // float twiddleExponent = mod(y * float(N)/ pow(2, currStage + 1), N);
    // twiddleExponent = 2.0 * M_PI * twiddleExponent / float(N);
    float twiddleExponent = y & int(pow(2, currStage + 1) - 1);
    twiddleExponent = -M_PI * float(twiddleExponent) / float(N);
    return vec2(cos(twiddleExponent), sin(twiddleExponent));
}

void main(){
    ivec2 pos = ivec2( gl_GlobalInvocationID.xy );
    vec4 inputVec1;
    vec4 inputVec2;
    int span = int(pow(2, currStage));
    vec2 twiddle = twiddleCompute(pos.y);
    // vec4 butterflyIndices_T = imageLoad(butterfly, ivec2(stage, pos.x));
    int firstIdx;  
    int secondIdx;
    int topOrBottom = int(mod(pos.y, pow(2,pos.y+1))); //pos.y & int(pow(2, currStage + 1) - 1) ;
    if(currStage == 0){
        if(topOrBottom < pow(2, currStage)){
            firstIdx = revInd[pos.y];
            secondIdx = revInd[pos.y + 1];
        }else{
            firstIdx = revInd[pos.y - 1];
            secondIdx = revInd[pos.y];
        }
    }else{
        if(topOrBottom < pow(2, currStage)){
            firstIdx = pos.y;
            secondIdx = pos.y  + int(pow(2, currStage));
        }
        else{
            firstIdx = pos.y - int(pow(2, currStage));
            secondIdx = pos.y;
        }   
    }
    if(inOutDecide == 0){
        inputVec1 = imageLoad(inOutTex0, ivec2(pos.x, firstIdx));
        inputVec2 = imageLoad(inOutTex0, ivec2(pos.x, secondIdx));
    }else{
        inputVec1 = imageLoad(inOutTex1, ivec2(pos.x, firstIdx));
        inputVec2 = imageLoad(inOutTex1, ivec2(pos.x, secondIdx));
    }
    vec2 res = inputVec1.xy;
    if(topOrBottom < pow(2, currStage))
        res += multiply(twiddle, inputVec2.xy); 
    else{
        res -= multiply(twiddle, inputVec2.xy); 
    }

    if(inOutDecide == 0){
        imageStore(inOutTex1, pos, vec4(res, res));
    }else{
        imageStore(inOutTex0, pos, vec4(res, res));
    }




}


/*
#version 430
#define M_PI 3.14159265358979323846264338327950288


layout(local_size_x = 16, local_size_y =16) in;
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

vec2 twiddleCompute(int y){
    float twiddleExponent = mod(y * float(N)/ pow(2, currStage + 1), N);
    twiddleExponent = 2.0 * M_PI * twiddleExponent / float(N);
    // float twiddleExponent = 2.0 * M_PI * y / float(pow(2, currStage + 1)); 
    return vec2(cos(twiddleExponent), sin(twiddleExponent));
}

void main(){
    ivec2 pos = ivec2( gl_GlobalInvocationID.xy );
    vec4 inputVec1;
    vec4 inputVec2;
    int span = int(pow(2, currStage));
    vec2 twiddle = twiddleCompute(pos.y);
    // vec4 butterflyIndices_T = imageLoad(butterfly, ivec2(stage, pos.x));
    int firstIdx;  
    int secondIdx;
    int topOrBottom = int(mod(pos.y, pow(2,pos.y+1))); //pos.y & int(pow(2, currStage + 1) - 1) ;
    if(currStage == 0){
        if(topOrBottom < pow(2, currStage)){
            firstIdx = revInd[pos.y];
            secondIdx = revInd[pos.y + 1];
        }else{
            firstIdx = revInd[pos.y - 1];
            secondIdx = revInd[pos.y];
        }
    }else{
        if(topOrBottom < pow(2, currStage)){
            firstIdx = pos.y;
            secondIdx = pos.y  + int(pow(2, currStage));
        }
        else{
            firstIdx = pos.y - int(pow(2, currStage));
            secondIdx = pos.y;
        }   
    }
    if(inOutDecide == 0){
        inputVec1 = imageLoad(inOutTex0, ivec2(pos.x, firstIdx));
        inputVec2 = imageLoad(inOutTex0, ivec2(pos.x, secondIdx));
    }else{
        inputVec1 = imageLoad(inOutTex1, ivec2(pos.x, firstIdx));
        inputVec2 = imageLoad(inOutTex1, ivec2(pos.x, secondIdx));
    }
    vec2 res = multiply(twiddle, inputVec2.xy) + inputVec1.xy;

    if(inOutDecide == 0){
        imageStore(inOutTex1, pos, vec4(res, res));
    }else{
        imageStore(inOutTex0, pos, vec4(res, res));
    }




}*/