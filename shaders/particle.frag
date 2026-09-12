#version 460 core

uniform vec3 cameraForward;

out vec4 fragColor;
flat in int InstanceID;

in vec3 vPos;

void main()
{
	if (length(vPos * 2) > 1)
	{
		discard;
	}
	fragColor = vec4(vPos + vec3(0.5), 1);
}