#version 430

layout(location = 0) in vec3 pos;		// Model-space position
layout(location = 1) in vec2 tex;		// Model-space normal

out vec3 fragNorm;
out vec3 fragPos;
// out float fog_factor;

uniform mat4 xform;			// Model-to-clip space transform
uniform float a_time;
uniform sampler2D dispMap;
uniform sampler2D normMap;
// uniform vec3 lightPos;
uniform vec3 camPos;
uniform vec2 windDir;
uniform float speed;


void main() {
	vec2 texCoord = pos.xy;
	vec2 further = texCoord / 16.0;
	vec2 evenFurther = texCoord / 64.0;


	
	float h = texture(dispMap, texCoord).b / 256.0 * 64.0;// ).r;
	float hFurther = texture(dispMap, further).b / 256.0 * 32.0;;// ).r;
	float hEvenFurther = texture(dispMap, evenFurther).b / 256.0 * 32.0;;// ).r;

	float xNew = texture(dispMap, texCoord).r / 256.0 * 16.0;// + windDir * speed).r;
	float xFurther = texture(dispMap, further).r / 256.0 * 32.0;;// ).r;
	float xEvenFurther = texture(dispMap, evenFurther).r / 256.0 * 32.0;;// ).r;

	float yNew = texture(dispMap, texCoord).g / 256.0 * 16.0;// + windDir * speed).r;
	float yFurther = texture(dispMap, further).g / 256.0 * 32.0;;// ).r;
	float yEvenFurther = texture(dispMap, evenFurther).g / 256.0 * 32.0;;// ).r;


	vec3 vtx = vec3(pos.x - xNew - xFurther - xEvenFurther,  pos.y - yNew - yFurther - yEvenFurther, h + hFurther + hEvenFurther); //+ texture(dispMap, texCoord).rgb;
	
	gl_Position =  xform* vec4(vtx,1.0);// + imageLoad(img_hk, pos).xyz, 1.0) ; 

	vec3 normal = texture(normMap, texCoord).rgb;
 
    vec4 v = xform * vec4(vtx, 1.0);
    vec3 normal1 = normalize(normal);
	fragPos = vtx.xyz;
    fragNorm = (inverse(transpose(xform)) * vec4(normal1, 0.0)).xyz;


}
