#version 430

// in vec4 posCurr;	// Interpolated model-space normal
layout(rgba32f, binding = 2) readonly uniform image2D img_hk;
in float heightVal;
in vec4 bump;
out vec4 outCol;	// Final pixel color

void main() {
	// Visualize normals as colors
	
	vec3 currCol = vec3(0.2,0.8,0.8);

	// // if(heightVal > 0.5*0.05){
	// // 	currCol = vec4(1.0, 1.0, 1.0, 1.0);
	// // }
	// // if(heightVal < 0.0){
	// // 	currCol = vec4(0.0, 0.0, 0.5, 1.0);
	// // }else if(heightVal > 0.0){
	// // 	currCol = vec4(0.0, 0.0, abs(heightVal * 100), 1.0);
	// // }
	float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 result = ambient * currCol;
	outCol = vec4(currCol,1.0);

	//vec4(imageLoad(img_hk, ivec2(gl_FragCoord.xyz)).rgba);
}
