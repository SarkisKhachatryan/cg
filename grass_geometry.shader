#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 4) out;

in VS_OUT{
    vec2 TexCoord;
} gs_in[];

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D grassDist;

void generateGrassObj(vec4 position, float size) {
    gl_Position = projection * view * model * vec4(position.x - size / 2, position.y + size, position.z, 1.0);
    TexCoords = vec2(0.0, 1.0);
    EmitVertex();

    gl_Position = projection * view * model * vec4(position.x - size / 2, position.y, position.z, 1.0);
    TexCoords = vec2(0.0, 0.0);
    EmitVertex();

    gl_Position = projection * view * model * vec4(position.x + size / 2, position.y + size, position.z, 1.0);
    TexCoords = vec2(1.0, 1.0);
    EmitVertex();

    gl_Position = projection * view * model * vec4(position.x + size / 2, position.y, position.z, 1.0);
    TexCoords = vec2(1.0, 0.0);
    EmitVertex();

    EndPrimitive();
}

void main() {

    vec4 centroidCoords = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position) / 3.0;
    vec2 pos = vec2((centroidCoords.x + 1) / 2, 1.0 - (centroidCoords.z + 1) / 2);
    vec4 texValue = texture(grassDist, pos);
    float size = 0.35;

    if (texValue.r > 0.95) {
        generateGrassObj(centroidCoords, size);
    }

}
