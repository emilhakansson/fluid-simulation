#version 430

struct Particle {
	vec3 position;
	vec3 predicted_position;
	vec3 velocity;
	float density;
	float near_density;
	float pad1;
	float pad2;
};

layout (std430, binding = 5) buffer SSBO {
	Particle particles[];
};

in VS_OUT {
	vec3 vertex;
	vec2 texcoords;
	vec3 velocity;
} fs_in;

flat in int InstanceID;
uniform bool velocity_shading;

out vec4 frag_color;

void main()
{
	if (length(fs_in.texcoords * 2 - 1) > 1) {
		discard;
	}

	vec3 base_color = vec3(0.25, 0.55, 1);

	if (velocity_shading) {
		float speed = length(particles[InstanceID].velocity);
		float s1 = 6.0f;
		float s2 = 12.0f;
		float s3 = 18.0f;
		float s4 = 36.0f;

		vec3 green = vec3(0.5, 1, 0.75);
		vec3 yellow = vec3(1, 1, 0);
		vec3 orange = vec3(1, 0.5, 0);
		vec3 red = vec3(1, 0.25, 0);

		vec3 velocity_color;
		if (speed < s1) {
			velocity_color = mix(base_color, green, speed / s1);
		} else if (speed < s2) {
			velocity_color = mix(green, yellow, (speed - s1) / (s2 - s1));
		} else if (speed < s3) {
			velocity_color = mix(yellow, orange, (speed - s2) / (s3 - s2));
		} else {
			velocity_color = mix(orange, red, (speed - s3) / (s4 - s3));
		}
		frag_color = vec4(velocity_color, 1);
	} else {
		frag_color = vec4(base_color, 1); //vec4(0.25, 0.75, 1, 1);
	}
}
