#version 430
#define M_PI 3.14159265358979323846264338327950288


layout(local_size_x = 1, local_size_y = 16) in;
layout(rgba32f, binding = 0) writeonly uniform image2D img_output;
layout(std430, binding = 1) buffer reverse_indices{
    int revInd[256];
};
uniform int N;


float twiddleExp_(ivec2 pos){
    float k = mod(pos.y * float(N)/ pow(2, pos.x + 1), N);
    return k;
}


void main(){
    ivec2 pos = ivec2( gl_GlobalInvocationID.xy ); 
    float twiddleExp = 2.0 * M_PI * twiddleExp_(pos) / float(N);
    vec2 twiddle = vec2(cos(twiddleExp), sin(twiddleExp));
    int bSpan = int(pow(2, pos.x));
    int topOrBottom;
    if(mod(pos.y, pow(2, pos.x + 1)) < bSpan){
        topOrBottom = 1; // Top
    }else{
        topOrBottom = 0;
    }

    // imageStore(img_output, pos, vec4(twiddle, pos));

    if(pos.x == 0){
        if(topOrBottom == 1)
            imageStore(img_output, pos, vec4(revInd[pos.y], revInd[pos.y + 1], twiddle.x, twiddle.y));
        else
            imageStore(img_output, pos, vec4(revInd[pos.y - 1], revInd[pos.y], twiddle.x, twiddle.y));
    }else{
        if(topOrBottom == 1)
            imageStore(img_output, pos, vec4(pos.y, pos.y + bSpan, twiddle.x, twiddle.y));
        else
            imageStore(img_output, pos, vec4(pos.y - bSpan, pos.y, twiddle.x, twiddle.y));
    }

}