#version 430

out vec4 outCol;	// Final pixel color

in vec3 fragNorm;
in vec3 fragPos;
uniform vec3 camPos;
vec3 lightPos = vec3(0.0, 0.0, 0.7);
 
void main (void) {
 	vec3 normalizedNorm = normalize(fragNorm);
	vec3 lightDirection =  normalize(lightPos); //normalize(lightPos - fragPos);// :
	vec3 viewDirection = normalize(camPos - fragPos);
	vec3 reflectedRay = 2 * dot(lightDirection,normalizedNorm)*normalizedNorm - lightDirection ;
	float diffuseTemp = max(dot(normalizedNorm, lightDirection), 0.0);
	float specularTemp = max(dot(viewDirection, reflectedRay), 0.0);
	float ambStr = 0.4;
	float diffStr = 0.4;
	float specStr = 0.5;
	float specExp = 8.0;
	vec3 lightCol = vec3(1.0);
	vec3 ambCol = ambStr * lightCol;
	vec3 difCol = diffStr * diffuseTemp * lightCol;
	vec3 speCol = specStr * pow(specularTemp, specExp) * lightCol;
	vec3 objColor = vec3(72.0/256.0, 191.0 / 256.0, 145.0 / 256.0);//, 1.0)
	outCol = vec4((difCol + ambCol + speCol) * objColor, 1.0);
    
}