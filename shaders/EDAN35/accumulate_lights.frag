#version 410

struct ViewProjTransforms
{
	mat4 view_projection;
	mat4 view_projection_inverse;
};

layout (std140) uniform CameraViewProjTransforms
{
	ViewProjTransforms camera;
};

layout (std140) uniform LightViewProjTransforms
{
	ViewProjTransforms lights[4];
};

layout (pixel_center_integer) in vec4 gl_FragCoord;

uniform int light_index;

uniform sampler2D depth_texture;
uniform sampler2D normal_texture;
uniform sampler2D shadow_texture;

uniform vec2 inverse_screen_resolution;

uniform vec3 camera_position;

uniform vec3 light_color;
uniform vec3 light_position;
uniform vec3 light_direction;
uniform float light_intensity;
uniform float light_angle_falloff;

layout (location = 0) out vec4 light_diffuse_contribution;
layout (location = 1) out vec4 light_specular_contribution;


void main()
{
	vec2 pixel_coord = gl_FragCoord.xy * inverse_screen_resolution;
	vec2 shadowmap_texel_size = 1.0f / textureSize(shadow_texture, 0);

	vec4 normal = texture(normal_texture, pixel_coord) * 2 - 1; // convert normal from rgb to tangent space
	vec3 n = normalize(normal.xyz);

	float d = texture(depth_texture, pixel_coord).r; 
	vec4 pixel_projection_position = vec4(pixel_coord * 2 - 1, d * 2 - 1, 1); // convert pixel coordinates from screen space to projection space
	vec4 pixel_view_position = (camera.view_projection_inverse * pixel_projection_position); // projection to view
	vec3 pixel_position = pixel_view_position.xyz / pixel_view_position.w; // perspective divide

	vec4 pixel_light_projection_position = lights[light_index].view_projection * pixel_view_position;
	pixel_light_projection_position *= 1/pixel_light_projection_position.w;
	vec3 pixel_light_position = pixel_light_projection_position.xyz * 0.5 + 0.5;

	int sample_grid_size = 15; // should be an odd number to keep light intensity constant
	int min_offset = -sample_grid_size / 2;
	int max_offset = sample_grid_size / 2;
	int nbr_samples = sample_grid_size * sample_grid_size;
	float bias = min(0.00001 * sample_grid_size * (1.0 - dot(n, normalize(light_direction))), 0.00005 * sample_grid_size); 
	float shadow = 0; // 0 = no light, 1 = fully lit

	for (int i = min_offset; i <= max_offset; i++) {
		for (int j = min_offset; j <= max_offset; j++) {
			ivec2 offset = ivec2(i, j);
			float light_depth = texture(shadow_texture, pixel_light_position.xy + offset * shadowmap_texel_size).r;
			if (pixel_light_position.z - bias < light_depth) {
				// the pixel is in shadow
				shadow += 1;
			}
		}
	}
	shadow /= nbr_samples;

	vec3 V = normalize(pixel_position - camera_position);
	vec3 L = normalize(light_position - pixel_position);
	vec3 R = normalize(reflect(L, n));
	

	float dist = distance(light_position, pixel_position);
	float distance_falloff = 1 / (1 + dist * dist);
	// For two 3D vectors a, b: cos(theta) = dot(a, b) / ( ||a|| * ||b|| )
	float cos_theta = dot(light_direction, -L)/(length(light_direction)*length(L));
	float angular_falloff = clamp((cos_theta - 0.8)/0.2, 0, 1);
	float intensity = angular_falloff * distance_falloff * light_intensity * shadow;

	vec3 diffuse = light_color * max(dot(n, L), 0) * intensity;

	float shininess = 15;
	float specular_intensity = 0.1 * pow(max(dot(R, V), 0), shininess) * intensity;
	vec3 specular = specular_intensity  * vec3(1, 1, 1);

	light_diffuse_contribution  = vec4(diffuse, 1);
	light_specular_contribution = vec4(specular, 1.0);
}
