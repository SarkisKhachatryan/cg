#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;

// ground texture
uniform sampler2D textureMainGround;
uniform sampler2D heightMap;

uniform vec3 lightPos = vec3(-10.0f, 1.0f, 2.0f);
uniform vec3 viewPos;
uniform vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

void main() {
    FragColor = texture(textureMainGround, TexCoords);
}



