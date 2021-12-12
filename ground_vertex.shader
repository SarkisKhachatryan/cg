#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texture_coords;

out VS_OUT{
	vec2 TexCoord;
} vertex_shader_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D textureMainGround;
uniform sampler2D heightMap;

void main() {
	gl_Position = vec4(position.x, (position.y + texture(heightMap, texture_coords).r) - 0.5, position.z, 1.0f);
	vertex_shader_out.TexCoord = texture_coords;
}