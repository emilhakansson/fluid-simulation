#version 460 core

uniform vec3 cameraForward;

out vec4 fragColor;
flat in int InstanceID;

in vec3 vPos;

void main()
{
	if (length(vPos * 2 - 1) > 1.4)
	{
		discard;
	}
	fragColor = vec4(vPos, 1);
}