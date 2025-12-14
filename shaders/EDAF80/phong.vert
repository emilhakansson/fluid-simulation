#version 410

layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
layout (location=2) in vec2 aTexcoords;
layout (location=3) in vec3 aTangent;
layout (location=4) in vec3 aBinormal;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;

uniform float time;


out vec3 position;
out vec3 normal;
out vec3 tangent;
out vec3 binormal;
out vec2 texcoords;

void main()
{
	position = vec3(vertex_model_to_world * vec4(aPos, 1));
	normal = (normal_model_to_world * vec4(aNormal, 0.0f)).xyz;
	tangent = aTangent;
	binormal = aBinormal;
	texcoords = aTexcoords;
	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(aPos,	1.0);

}

