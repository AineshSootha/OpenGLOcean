#version 430
#define M_PI 3.14159265358979323846264338327950288


layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba32f, binding = 0) writeonly uniform image2D img_output;
layout(binding = 1) uniform sampler2D randSampler1;

uniform int N;
uniform int startL;
uniform int V;
uniform int A;
uniform float g = 9.81;
uniform vec2 windDir;

vec2 gaussianRandoms(){
    vec2 texCoord = vec2(gl_GlobalInvocationID.xy) / float(N);
    float random1 = clamp(texture(randSampler1, texCoord).r, 0.001, 1.0);
    float random2 = clamp(texture(randSampler1, texCoord).g, 0.001, 1.0);
    float factor1 = sqrt(-2*log(random1));
    float factor2 = 2*M_PI*random2;
    return vec2(factor1 * cos(factor2), factor1 * sin(factor2)); 
}


void main() {
  ivec2 pos = ivec2( gl_GlobalInvocationID.xy);    // get value stored in the image
  float n = pos.x > float(N) / 2 ? pos.x - float(N) :  pos.x;
  float m = pos.y > float(N) / 2 ? pos.y - float(N) :  pos.y;
  /* Not sure why SegFaults if using xDash yDash
  float xDash = (pos.x - float(N) / 2); //Subtract N/2 otherwise range is 0->N
  float yDash = (pos.y - float(N) / 2);
  */
  vec2 k = 2.0 * M_PI * vec2(n, m) / startL; 
  vec2 gaussianRands = gaussianRandoms();
  float L = V * V / g;
  float magK = length(k) < 0.0001 ? 0.0001 : length(k);

  float Phk = 2 * exp(-1.0 / pow(magK * L, 2.0)) * exp(pow(-magK * 0.1, 2.0)) * pow(dot(normalize(-k), normalize(windDir)), 8.0) / (pow(magK, 4));
  float h0k = clamp(sqrt(Phk) / sqrt(2.0), -400, 400);  
  imageStore( img_output, pos, vec4(gaussianRands.xy*h0k,  magK, 1.0 ) );
}
