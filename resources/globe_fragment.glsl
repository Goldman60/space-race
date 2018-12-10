#version 330 core
out vec4 color;
in vec3 fragNor;
in vec4 onlyviewpos;
in vec4 zpos;
void main()
{
	vec3 normal = normalize(fragNor);
	color.rgb = vec3(1,1,1);
    color.a = 0.2;
}
