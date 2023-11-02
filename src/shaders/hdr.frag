#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform int hdrMode;
uniform bool bloomOn;
uniform float exposure;
uniform sampler2D hdrBuffer;
uniform sampler2D bloom;

void main(){
    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;

    if(bloomOn){
        vec3 bloomColor = texture(bloom, TexCoords).rgb;
        hdrColor += bloomColor;
    }

    if(hdrMode == 0)
        FragColor = vec4(hdrColor, 1.0);
    else if(hdrMode == 1){
         // reinhard tone mapping
        vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
        // gamma correction 
        mapped = pow(mapped, vec3(1.0 / gamma));
  
        FragColor = vec4(mapped, 1.0);
    } else if(hdrMode == 2){       
        vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
        mapped = pow(mapped, vec3(1.0 / gamma));

        FragColor = vec4(mapped, 1.0);
    }
}