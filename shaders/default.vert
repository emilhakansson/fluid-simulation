#version 460 core

layout(location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 viewProjection;

out vec3 aPos;

void main()
{
	aPos = pos;
	gl_Position = viewProjection * model * vec4(aPos, 1.0);
}