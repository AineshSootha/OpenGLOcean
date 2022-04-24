#version 430

layout(location = 0) in vec3 pos;		// Model-space position
layout(location = 1) in vec2 tex;		// Model-space normal
out float heightVal;
// layout(rgba32f, binding = 1) writeonly uniform image2D img_output;
vec3 curPos;
// out vec3 col;	// Model-space interpolated normal
// layout(rgba32f, binding = 2) readonly uniform image2D img_hk;
uniform mat4 xform;			// Model-to-clip space transform
uniform float a_time;
uniform sampler2D dispMap;
uniform vec2 windDir = normalize(vec2(1.0,1.0));
uniform float speed;
// layout(rgba32f, binding = 2) readonly uniform image2D img_hk;

void main() {
	// Transform vertex position
	// curPos = view * pos;
	
	// vec2 texCoord = pos.xy / float(256);
	// vec2 texCoord = vec2(pos.xy) / float(256);
	float h = texture(dispMap, pos.xy + windDir * speed).r / 64.0;
	heightVal = h < 0 ? 0.0 : h;
	vec3 vtx = vec3(pos.x,  pos.y  , pos.z+ h); //+ texture(dispMap, texCoord).rgb;
	// vtx.y = (sin(2.0 * vtx.x + a_time/1000.0 ) * cos(1.5 * vtx.y + a_time/1000.0) * 0.2);
	gl_Position =  xform* vec4(vtx,1.0);// + imageLoad(img_hk, pos).xyz, 1.0) ; 

	// if(pos.x > -0.5 && pos.x < 0.5 && pos.y > -0.5 && pos.y < 0.5){
	// col = vec3(1.0,1.0,1.0);
	//}
}
