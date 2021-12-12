#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 tCoord;

out VS_OUT{
    vec2 texCoord;
} vertex_shader_out;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D heightMap;

void main() {
    gl_Position = vec4(position.x, texture(heightMap, tCoord).r, position.z, 1.0);
    vertex_shader_out.texCoord = tCoord;
}
