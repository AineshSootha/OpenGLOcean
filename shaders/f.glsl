#version 430

// in vec4 posCurr;	// Interpolated model-space normal
layout(rgba32f, binding = 2) readonly uniform image2D img_hk;
out vec4 outCol;	// Final pixel color

void main() {
	// Visualize normals as colors

	outCol = vec4(imageLoad(img_hk, ivec2(gl_FragCoord.xyz)).rgba);
}
