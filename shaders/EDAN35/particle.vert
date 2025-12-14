#version 430

struct Particle {
	vec3 position;
	vec3 predicted_position;
	vec3 velocity;
	float density;
	float pad;
	float pad2;
	float pad3;
};

layout (std430, binding = 5) buffer SSBO {
	Particle particles[];
};

layout (location = 0) in vec3 vertex;
layout (location = 2) in vec3 texcoords;

uniform vec3 camera_up;
uniform vec3 camera_right;
uniform float particle_scale;
uniform mat4 vertex_model_to_world;
uniform mat4 vertex_world_to_clip;

uniform vec3 camera_position;

out VS_OUT {
	vec3 vertex;
	vec2 texcoords;
	vec3 velocity;
} vs_out;

flat out int InstanceID;

void main()
{
	InstanceID = gl_InstanceID;
	vec3 vertex_billboard = camera_right * vertex.x + camera_up * vertex.y;
	vs_out.vertex = vertex_billboard;
	vs_out.texcoords = texcoords.xy;
	vs_out.velocity = particles[gl_InstanceID].velocity;
	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4((vertex_billboard * particle_scale + particles[gl_InstanceID].position), 1.0);
}
