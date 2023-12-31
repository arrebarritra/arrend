#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D image;

uniform bool horizontal;
uniform float weights[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main(){
    vec2 tex_offset = 1.0 / textureSize(image, 0);
    vec3 result = texture(image, TexCoords).rgb * weights[0];
    if(horizontal){
        for(int i = 1; i < 5; i++){
            result += texture(image, TexCoords + i * tex_offset * vec2(1.0, 0.0)).rgb * weights[i];
            result += texture(image, TexCoords - i * tex_offset * vec2(1.0, 0.0)).rgb * weights[i];
        }
    } else {
        for(int i = 1; i < 5; i++){
            result += texture(image, TexCoords + i * tex_offset * vec2(0.0, 1.0)).rgb * weights[i];
            result += texture(image, TexCoords - i * tex_offset * vec2(0.0, 1.0)).rgb * weights[i];
        }
    }

    FragColor = vec4(result, 1.0);
}