#version 330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
out vec3 fragNor;
out vec4 onlyviewpos;
out vec4 zpos;
void main()
{
    onlyviewpos = V * vec4(vertPos,0.0);
    zpos = P * V * M * vec4(vertPos,0.0);
	fragNor = vec4(M * vec4(vertNor, 0.0)).xyz;
	gl_Position = P * V * M * vec4(vertPos, 1.0);
}
