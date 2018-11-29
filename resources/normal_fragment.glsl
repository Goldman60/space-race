#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;

uniform sampler2D tex;
uniform sampler2D tex2;

void main()
{
vec3 normal = normalize(vertex_normal);
color = vec4(normal, 1);
}
