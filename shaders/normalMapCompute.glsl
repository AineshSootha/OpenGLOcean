#version 430

layout(local_size_x = 16, local_size_y = 16) in;
layout (binding = 0, rgba32f) readonly uniform image2D dispMap; 
layout (binding = 1, rgba32f) writeonly uniform image2D normMap;


void main(){
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    float dx = (imageLoad(dispMap, ivec2(pos.x+1, pos.y)).b;
    float dy = iamgeLoad(dispMap, ivec2(pos.x-1, pos.y)).b;
    vec3 direction = vec3(-dx,-dy,1.0);
    vec3 normal = normalize(direction);
    imageStore(normMap, pos, vec4(normal, 1.0));
}
