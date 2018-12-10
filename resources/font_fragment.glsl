#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;

uniform sampler2D tex;
uniform int digit;

void main()
{
vec4 tcol = texture(tex, vec2((vertex_tex.x / 16.0) + (digit/16.0) , (vertex_tex.y / 8.0) + (1.0/8.0)));
color = tcol;
}
