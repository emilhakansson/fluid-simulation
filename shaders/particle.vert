#version 460 core

struct Particle
{
	vec3 position;
	vec3 velocity;
	float density;
};

layout(location = 0) in vec3 vertex;
layout(std430, binding = 5) buffer SSBO
{
	Particle particles[];
};

uniform mat4 viewProjection;
uniform mat4 model;
uniform vec3 cameraForward;
uniform vec3 cameraRight;
uniform float particleScale;


flat out int InstanceID;
out vec3 vPos;
out float density;

void main()
{
	InstanceID = gl_InstanceID;
	density = particles[InstanceID].density;
	vec3 cameraUp = cross(cameraForward, cameraRight);
	vPos = vertex;
	vec3 pos = (vertex.x * cameraRight + vertex.y * cameraUp) * particleScale;
	gl_Position = viewProjection * model * vec4(pos + particles[InstanceID].position, 1);
}