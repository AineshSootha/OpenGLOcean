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


    float dxx = imageLoad(dispMapX, ivec2(pos.x + 1, pos.y)).r - normalMap.x;
    float dxy = imageLoad(dispMapX, ivec2(pos.x, pos.y + 1)).r - normalMap.x;
    normalMap.x = sqrt(dxx * dxx + dxy * dxy);


    float dyx = imageLoad(dispMapY, ivec2(pos.x + 1, pos.y)).r - normalMap.y;
    float dyy = imageLoad(dispMapY, ivec2(pos.x, pos.y + 1)).r - normalMap.y;
    normalMap.y = sqrt(dyx * dyx + dyy * dyy);

    float dzx = imageLoad(dispMapZ, ivec2(pos.x + 1, pos.y)).r - normalMap.z;
    float dzy = imageLoad(dispMapZ, ivec2(pos.x, pos.y + 1)).r - normalMap.z;
    normalMap.z = sqrt(dzx * dzx + dzy * dzy);
    
    

    normalMap.w = 1.0;
    imageStore(finalOut, pos, normalMap);
}

