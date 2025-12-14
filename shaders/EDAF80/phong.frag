#version 410

uniform vec3 sun_position;
uniform vec3 ambient_colour;
uniform vec3 diffuse_colour;
uniform vec3 specular_colour;
uniform float shininess_value;
uniform vec3 camera_position;
uniform float shininess;

uniform sampler2D diffuse_texture;
uniform sampler2D normal_map;
uniform sampler2D specular_map;

uniform int use_normal_mapping;

in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec3 binormal;
in vec2 texcoords;

out vec4 f_color;

void main()
{
	vec4 diffuse_tex = texture(diffuse_texture, texcoords);
	vec4 specular_tex = texture(specular_map, texcoords);

	vec3 n;
	if (use_normal_mapping == 1)
	{
		mat3 TBN = mat3(tangent, binormal, normal);
		n = normalize(TBN * vec3(texture(normal_map, texcoords)*2 - 1));
	} else
	{
		n = normalize(normal);
	}

	vec3 L = normalize(sun_position - position);
	vec3 V = normalize(camera_position - position);
	vec3 R = normalize(reflect(-L, n));

	vec4 ambient = vec4(ambient_colour, 0);
	vec4 diffuse = diffuse_tex * vec4(diffuse_colour, 0) * max(dot(n, L), 0);
	vec4 specular = specular_tex * vec4(specular_colour * pow(max(dot(R, V),0), shininess_value), 0);
	f_color = vec4(0.02, 0.01, 0.05, 1) + vec4(1.0, 0.0, 0, 1) * max(dot(n, L), 0);// diffuse; //+ specular;
}

