#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;

uniform sampler2D tex6;
uniform sampler2D tex7;
uniform sampler2D tex8;
uniform sampler2D tex9;

void main()
{
vec3 n = normalize(vertex_normal);
vec3 lp=vec3(10,-20,-100);
vec3 ld = normalize(vertex_pos - lp);
float diffuse = dot(n,ld);

color = texture(tex7,  vec2(vertex_tex.x, -1 * vertex_tex.y)).rgba;
color += texture(tex6, vec2(vertex_tex.x, -1 * vertex_tex.y)).rgba;
color *= diffuse*0.7 + ((1 - diffuse*0.7) * texture(tex9,  vec2(vertex_tex.x, -1 * vertex_tex.y)).rgba);

vec3 cd = normalize(vertex_pos - campos);
vec3 h = normalize(cd+ld);
float spec = dot(n,h);
spec = clamp(spec,0,1);
spec = pow(spec,200);
color += vec4(vec3(texture(tex8, vec2(vertex_tex.x, -1 * vertex_tex.y)))*spec*3, 1);



}
