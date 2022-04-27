#version 430
#define M_PI 3.14159265358979323846264338327950288


layout(local_size_x = 16, local_size_y = 16) in;
// layout(rgba32f, binding = 0) writeonly uniform image2D img_output;
layout(rgba32f, binding = 1) uniform image2D inOutTex0;
layout(rgba32f, binding = 2) uniform image2D inOutTex1;
layout(rgba32f, binding = 0) uniform image2D finalOut;

uniform int N;
uniform int inOutDecide;


void main(){
    ivec2 pos = ivec2( gl_GlobalInvocationID.xy );
    // float inversionSign[] = {1.0, -1.0}; 
    float inversion =  ((pos.x + pos.y) & 1) == 1 ? -1.0 : 1.0;
    if(inOutDecide == 0){
        float inH = imageLoad(inOutTex0, pos).r;
        imageStore(finalOut, pos, vec4(inversion * inH / (N*N), inversion * inH / (N*N), inversion * inH / (N*N),1.0));
    }else{
        float inH = imageLoad(inOutTex1, pos).r;
       imageStore(finalOut, pos, vec4(inversion * inH / (N*N), inversion * inH / (N*N), inversion * inH / (N*N),1.0));
    }
}


        // imageStore(finalOut, pos, vec4(inversion * inH / (N*N), inversion * inH / (N*N), inversion * inH / (N*N),1.0));
 