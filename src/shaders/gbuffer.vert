#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;


out vec3 FragPos;
out vec2 TexCoords;
out mat3 TBN;

layout (std140) uniform Matrices{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;

void main(){
	gl_Position = projection * view * model * vec4(aPos, 1.0);

	FragPos = vec3(view * model * vec4(aPos, 1.0));
	
	mat3 normalMatrix =  transpose(inverse(mat3(view * model)));
	vec3 Normal = normalize(normalMatrix * aNormal);
	vec3 Tangent = normalize(normalMatrix * aTangent);
	vec3 Bitangent = normalize(normalMatrix * aBitangent);
	TBN = mat3(Tangent, Bitangent, Normal); //tangent space to world space

	TexCoords = aTexCoords;
}