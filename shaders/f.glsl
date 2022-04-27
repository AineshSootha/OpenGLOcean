#version 430

// in vec4 posCurr;	// Interpolated model-space normal
layout(rgba32f, binding = 2) readonly uniform image2D img_hk;
in float heightVal;
in vec4 bump;
out vec4 outCol;	// Final pixel color

void main() {
	// Visualize normals as colors
	vec4 currCol = vec4(0.3,1.0,bump.w,1.0);
	// // if(heightVal > 0.5*0.05){
	// // 	currCol = vec4(1.0, 1.0, 1.0, 1.0);
	// // }
	// // if(heightVal < 0.0){
	// // 	currCol = vec4(0.0, 0.0, 0.5, 1.0);
	// // }else if(heightVal > 0.0){
	// // 	currCol = vec4(0.0, 0.0, abs(heightVal * 100), 1.0);
	// // }
	outCol = currCol;

	//vec4(imageLoad(img_hk, ivec2(gl_FragCoord.xyz)).rgba);
}
