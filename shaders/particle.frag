#version 460 core

uniform vec3 cameraForward;
uniform float targetDensity;

out vec4 fragColor;
flat in int InstanceID;

in vec3 vPos;
in float density;

void main()
{
	if (length(vPos * 2) > 1)
	{
		discard;
	}
	vec3 red = vec3(1, 0, 0);
	vec3 blue = vec3(0.25, 0.5, 1);
	vec3 white = vec3(1, 1, 1);

	float error = density - targetDensity;
	float margin = 1.0f;
	vec3 color = white;

	if (error > 0.0f)
	{
		color = mix(white, red, error);
	}
	else if (error < 0.0f)
	{
		color = mix(white, blue, abs(error));
	}

	fragColor = vec4(color, 1);
}