#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout(rgba32f, binding = 0) uniform image2D finalOut;
layout(rgba32f, binding = 1) uniform image2D dispMapX;
layout(rgba32f, binding = 2) uniform image2D dispMapY;
layout(rgba32f, binding = 3) uniform image2D dispMapZ;


void main(){
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    vec4 normalMap;
    normalMap.x = imageLoad(dispMapX,pos).r;
    normalMap.y = imageLoad(dispMapY,pos).r;
    normalMap.z = imageLoad(dispMapZ,pos).r;
    normalMap.w = 1.0;
    imageStore(finalOut, pos, normalMap);
}

