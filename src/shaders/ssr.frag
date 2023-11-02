#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

layout (std140) uniform Matrices{
    mat4 projection;
    mat4 view;
};
float maxDistance = 3;
float resolution = 0.1;
float maxSteps = maxDistance / resolution;
int bsSteps = 50;
float thickness = 0.05;

vec3 RayCast(vec3 origin, vec3 dir, out float dDepth);
vec3 BinarySearch(vec3 start, vec3 dir, out float dDepth);
vec2 viewToUV(vec4 pos);

void main(){
	vec3 fragPos = texture(gPosition, TexCoords).xyz;
	vec3 fragDir = normalize(fragPos);
	vec3 normal = texture(gNormal, TexCoords).xyz;
	vec3 reflectDir = reflect(fragDir, normal);

	float dDepth;
	FragColor = vec4(RayCast(fragPos, reflectDir, dDepth), 1.0);
	// store visibility in z coord of output
	FragColor.z *=  (1 - max(dot(-fragDir, reflectDir), 0.0)) * (1 - clamp(dDepth / thickness, 0.0, 1.0)) *
			(1 - clamp(length((texture(gPosition, FragColor.xy).xyz - fragPos)) / maxDistance, 0.0, 1.0)) *
			(FragColor.x < 0 || FragColor.x > 1 ? 0 : 1) * (FragColor.y < 0 || FragColor.y > 1 ? 0 : 1) *
			16 * FragColor.x * (1 - FragColor.x) * FragColor.y * (1 - FragColor.y);
	FragColor.z = clamp(FragColor.z, 0.0, 1.0);
}

// return uv of hit
vec3 RayCast(vec3 origin, vec3 dir, out float dDepth){
	float sceneDepth;
	vec2 projectedCoord;

	vec3 hitCoord = origin;
	for(int i = 0; i < maxSteps; i++){
		hitCoord += dir * resolution;
		projectedCoord = viewToUV(vec4(hitCoord, 1.0));
		sceneDepth = texture(gPosition, projectedCoord.xy).z;

		float dDepth = sceneDepth - hitCoord.z;
		if(dDepth > 0.0 && dDepth <= thickness){
			return BinarySearch(hitCoord, dir, dDepth);
		}
	}
	return vec3(0.0);
}

vec3 BinarySearch(vec3 start, vec3 dir, out float dDepth){
	float sceneDepth;
	vec2 projectedCoord;

	vec3 hitCoord = start;
	for(int i = 0; i < bsSteps; i++){
		projectedCoord = viewToUV(vec4(hitCoord, 1.0));
		sceneDepth = texture(gPosition, projectedCoord).z;
		
		float dDepth = sceneDepth - hitCoord.z;
		resolution *= 0.5;
		hitCoord += -sign(dDepth) * dir * resolution;
	}

	return vec3(viewToUV(vec4(hitCoord, 1.0)), 1.0);
}

vec2 viewToUV(vec4 pos){
	pos = projection * pos;
	pos.xy /= pos.w;
	pos.xy = pos.xy * 0.5 + 0.5;
	return pos.xy;
}