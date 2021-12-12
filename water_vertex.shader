#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform float time;

out vec3 Normal;
out vec4 FragPos;
out mat4 modelToPass;
out mat4 viewToPass;
out mat4 projectionToPass;
out float timeToPass;

out VS_OUT {
	vec2 TexCoord;
	int performWave;
} vertex_shader_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool performWave;

void main() {
	modelToPass = model;
	viewToPass = view;
	projectionToPass = projection;
	timeToPass = time;

	if (performWave) {
		vertex_shader_out.performWave = 1;
	}

	gl_Position = vec4(aPos, 1.0f);
	vertex_shader_out.TexCoord = vec2(aTexCoord.x, aTexCoord.y);

	FragPos = vec4(aPos, 1.0);
	Normal = transpose(inverse(mat3(model))) * vec3(0, 1, 0);
}