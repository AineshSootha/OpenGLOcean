#version 430

layout(location = 0) in vec3 pos;		// Model-space position
layout(location = 1) in vec3 tex;		// Model-space normal
// layout(rgba32f, binding = 1) writeonly uniform image2D img_output;
vec3 curPos;
// out vec3 col;	// Model-space interpolated normal
// layout(rgba32f, binding = 2) readonly uniform image2D img_hk;
uniform mat4 xform;			// Model-to-clip space transform
uniform float a_time;
// layout(rgba32f, binding = 2) readonly uniform image2D img_hk;

void main() {
	// Transform vertex position
	// curPos = view * pos;
	// curPos.y = (sin(2.0 * pos.x + uTime/1000.0 ) * cos(1.5 * pos.y + uTime/1000.0) * 0.2);
	vec3 vtx = pos;
	// vtx.y = (sin(2.0 * vtx.x + a_time/1000.0 ) * cos(1.5 * vtx.y + a_time/1000.0) * 0.2);
	gl_Position = xform * vec4(vtx,1.0);// + imageLoad(img_hk, pos).xyz, 1.0) ; 
	// if(pos.x > -0.5 && pos.x < 0.5 && pos.y > -0.5 && pos.y < 0.5){
	// col = vec3(1.0,1.0,1.0);
	//}
}
