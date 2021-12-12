#version 330 core

in vec2 TexCoords;

out vec4 color;

uniform float Ka;
uniform float Kd;
uniform float Ks;

uniform sampler2D grassText;

void main() {
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 Ia = Ka * lightColor;

    color = texture(grassText, TexCoords);
}
