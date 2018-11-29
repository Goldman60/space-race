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
vec4 tcol = vec4(texture(tex, vertex_tex).rgb + 0.25, 1);
color = tcol;
}
