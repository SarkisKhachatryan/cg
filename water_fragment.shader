#version 330 core
out vec4 FragColor;

in GS_OUT {
    vec2 TexCoord;
} fragment_shader_in;

// water texture
uniform sampler2D textureMain;
uniform sampler2D DudvMap;
uniform sampler2D Reflection;

in vec3 NormalG;
in vec4 FragPosG;

uniform vec3 lightPos = vec3(-10.0f, 1.0f, 2.0f);
uniform vec3 viewPos;
uniform vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

uniform float offset;

void main() {
	vec2 distortion1 = (texture(DudvMap, fragment_shader_in.TexCoord).rg * 2.0 - 1.0) * 0.9;
    vec2 coord = fragment_shader_in.TexCoord * vec2(5,5);
	coord += vec2(offset, 0);
	coord += distortion1;

    // ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse 
    vec3 norm = normalize(NormalG);
    vec3 lightDir = normalize(lightPos - vec3(FragPosG));
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient) * vec3(0.0f, 0.0f, 1.0f);
    vec2 ss = (FragPosG.xy / FragPosG.w) * 0.5 + 0.5;

    FragColor = vec4(mix(texture(textureMain, coord), texture(Reflection, vec2(ss.x, -ss.y)), 0.5).rgb, 0.7);
}

