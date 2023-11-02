#version 410 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform sampler2D ssao;
uniform sampler2D ssr;
uniform bool ssaoOn;
uniform bool ssrOn;

layout (std140) uniform Matrices{
    mat4 projection;
    mat4 view;
};

struct PointLight {
	vec4 pos, color;
	float Linear, Quadratic, pad1, pad2;
};

struct DirLight {
	vec4 dir, color;
};

struct SpotLight {
	vec4 pos, dir, color;
	float cutOff, outerCutOff, Linear, Quadratic;
};

#define NR_POINT_LIGHTS 1
#define NR_DIR_LIGHTS 1
#define NR_SPOT_LIGHTS 1

layout (std140) uniform Lights{
	PointLight pointLights[NR_POINT_LIGHTS];
	DirLight dirLights[NR_DIR_LIGHTS];
	SpotLight spotLights[NR_SPOT_LIGHTS];
};


void main(){
    vec3 FragPos = texture(gPosition, TexCoords).xyz;
    vec3 Normal = texture(gNormal, TexCoords).xyz;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    
    vec3 viewDir = normalize(-FragPos);

    // ambient
    vec3 ambient;
    if(ssaoOn){
        float AmbientOcclusion = texture(ssao, TexCoords).r;
        ambient = 0.1 * AmbientOcclusion * Albedo;
    } else {
        ambient = 0.1 * Albedo;
    }

    vec3 lighting = ambient;

    for(int i = 0; i < NR_POINT_LIGHTS; i++){
        vec3 lightPos = vec3(view * pointLights[i].pos);
        vec3 lightDir = normalize(lightPos - FragPos);
        
        // diffuse
        float diff = max(dot(Normal, lightDir), 0.0);
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 32.0) * Specular;
        // attenuation
        float dist = length(lightPos - FragPos);
        float attenuation = 1.0 / (1.0 + pointLights[i].Linear * dist + pointLights[i].Quadratic * dist * dist);
        // result
        lighting += attenuation * (diff + spec) * Albedo * vec3(dirLights[i].color);
    }

    for(int i = 0; i < NR_DIR_LIGHTS; i++){
        vec3 lightDir = vec3(view * dirLights[i].dir);
        // diffuse
        float diff = max(dot(Normal, -lightDir), 0.0);
        // specular
        vec3 halfwayDir = normalize(-lightDir + viewDir);  
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 32.0) * Specular;
        // result
        lighting += (diff + spec) * Albedo * vec3(dirLights[i].color);
    }

    for(int i = 0; i < NR_SPOT_LIGHTS; i++){
        vec3 lightDir = vec3(view * spotLights[i].dir);
        vec3 lightPos = vec3(view * spotLights[i].pos);
        
        float theta = dot(lightDir, normalize(FragPos - lightPos));
        float epsilon = spotLights[i].cutOff - spotLights[i].outerCutOff;
        float intensity = clamp((theta - spotLights[i].outerCutOff) / epsilon, 0.0, 1.0);
        if(theta > spotLights[i].cutOff){
            // diffuse
            float diff = max(dot(Normal, vec3(-lightDir)), 0.0);
            // specular
            vec3 halfwayDir = normalize(vec3(-lightDir) + viewDir);  
            float spec = pow(max(dot(Normal, halfwayDir), 0.0), 32.0) * Specular;
            // result
            lighting += intensity * (diff + spec) * Albedo * vec3(spotLights[i].color);
        }
    }

    if(ssrOn){
        vec2 reflectionUV = texture(ssr, TexCoords).xy;
        float reflectionVis = texture(ssr, TexCoords).z;
        vec3 reflectionColor = texture(gAlbedoSpec, reflectionUV).rgb;
        lighting += reflectionColor * reflectionVis * Specular;
    }

    FragColor = vec4(lighting, 1.0);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    BrightColor = vec4(FragColor.rgb * vec3(brightness > 1.0), 1.0);
}