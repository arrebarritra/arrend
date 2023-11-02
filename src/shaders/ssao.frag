#version 410 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D noiseTexture;

layout (std140) uniform Matrices{
    mat4 projection;
    mat4 view;
};

const int kernelSize = 64;
uniform vec3 samples[kernelSize];

const float radius = 0.5;
const float bias = 0.025;
const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0);

void main(){
	vec3 fragPos = texture(gPosition, TexCoords).xyz;
	vec3 normal = texture(gNormal, TexCoords).rgb;
	vec3 randomVec = normalize(texture(noiseTexture, TexCoords * noiseScale).xyz); 

	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; i++){
		vec3 samplePos = fragPos + TBN * samples[i] * radius; // view space sample pos
		
		vec4 offset = projection * vec4(samplePos, 1.0);
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;
		vec3 sampleTexturePos = texture(gPosition, offset.xy).xyz;
		float sampleDepth = sampleTexturePos.z;

		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0);
	}
	occlusion = 1.0 - (occlusion / kernelSize);
	FragColor = occlusion;
}