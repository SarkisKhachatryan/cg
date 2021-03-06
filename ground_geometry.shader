#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 TexCoord;
} geometry_shader_in[];

out vec2 TexCoords;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

vec3 getNormal() {
    vec3 xy = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 yx = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(yx, xy));
}

void main() {
    Normal = transpose(inverse(mat3(model))) * getNormal();

    gl_Position = projection * view * model * gl_in[0].gl_Position;
    TexCoords = geometry_shader_in[0].TexCoord;
    EmitVertex();

    gl_Position = projection * view * model * gl_in[1].gl_Position;
    TexCoords = geometry_shader_in[1].TexCoord;
    EmitVertex();

    gl_Position = projection * view * model * gl_in[2].gl_Position;
    TexCoords = geometry_shader_in[2].TexCoord;
    EmitVertex();

    EndPrimitive();// finilaze triangle
}