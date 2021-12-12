#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out VS_OUT{
	vec2 TexCoord;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


uniform sampler2D textureMainGround;
uniform sampler2D heightMap;

void main()
{
	gl_Position = vec4(aPos.x, (aPos.y + texture(heightMap, aTexCoord).r) - 0.5, aPos.z, 1.0f);
	vs_out.TexCoord = aTexCoord;
}