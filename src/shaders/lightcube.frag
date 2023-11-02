#version 410 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct PointLight {
	vec4 pos, color;
	float Linear, Quadratic, pad1, pad2;
};

struct DirLight {
	vec4 dir, color;
};

struct SpotLight {
	vec3 pos, dir, color;
	float cutoff, Linear, Quadratic, pad3;
};

#define NR_POINT_LIGHTS 100
#define NR_DIR_LIGHTS 2
#define NR_SPOT_LIGHTS 100

layout (std140) uniform Lights{
	PointLight pointLights[NR_POINT_LIGHTS];
	DirLight dirLights[NR_DIR_LIGHTS];
	SpotLight spotLights[NR_SPOT_LIGHTS];
};

uniform uint lightIndex;

void main(){
    FragColor = vec4(vec3(pointLights[lightIndex].color), 1.0);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    BrightColor = vec4(FragColor.rgb * vec3(brightness > 1.0), 1.0);
}