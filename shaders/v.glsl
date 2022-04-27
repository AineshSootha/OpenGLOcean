#version 430

layout(location = 0) in vec3 pos;		// Model-space position
layout(location = 1) in vec2 tex;		// Model-space normal
out float heightVal;
out vec4 bump;
// layout(rgba32f, binding = 1) writeonly uniform image2D img_output;
vec3 curPos;
// out vec3 col;	// Model-space interpolated normal
// layout(rgba32f, binding = 2) readonly uniform image2D img_hk;
uniform mat4 xform;			// Model-to-clip space transform
uniform float a_time;
uniform sampler2D dispMapZ;
uniform sampler2D dispMapX;
uniform sampler2D dispMapY;
uniform sampler2D dispMap;
uniform vec2 windDir;
uniform float speed;

const ivec3 off = ivec3(-1,0,1);
const vec2 size = vec2(2.0,0.0);

// layout(rgba32f, binding = 2) readonly uniform image2D img_hk;

void main() {
	// Transform vertex position
	// curPos = view * pos;
	
	// vec2 texCoord = pos.xy / float(256);
	// vec2 texCoord = vec2(pos.xy) / float(256);
	vec2 texCoord = (pos.xy);
	float h = texture(dispMap, texCoord + windDir * speed).b * 0.05;// ).r;
	float xNew = texture(dispMap, texCoord).r * 0.05;// + windDir * speed).r;
	float yNew = texture(dispMap, texCoord).g * 0.05;// + windDir * speed).r;
	vec2 tex_coord = pos.xy / 256.0;
	heightVal = h;
	vec3 vtx = vec3(pos.x - xNew,  pos.y -yNew, h); //+ texture(dispMap, texCoord).rgb;
	// vtx.y = (sin(2.0 * vtx.x + a_time/1000.0 ) * cos(1.5 * vtx.y + a_time/1000.0) * 0.2);
	gl_Position =  xform* vec4(vtx,1.0);// + imageLoad(img_hk, pos).xyz, 1.0) ; 

    float s11 = h;
    float s01 = textureOffset(dispMapZ, texCoord, off.xy).x;
    float s21 = textureOffset(dispMapZ, texCoord, off.zy).x;
    float s10 = textureOffset(dispMapZ, texCoord, off.yx).x;
    float s12 = textureOffset(dispMapZ, texCoord, off.yz).x;
    vec3 va = normalize(vec3(size.xy,s21-s01));
    vec3 vb = normalize(vec3(size.yx,s12-s10));
    bump = vec4( cross(va,vb), s11 );
	// if(pos.x > -0.5 && pos.x < 0.5 && pos.y > -0.5 && pos.y < 0.5){
	// col = vec3(1.0,1.0,1.0);
	//}
}
