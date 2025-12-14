#include <glm/glm.hpp>

struct Particle {
	glm::vec3 position;
	float pad_0;
	glm::vec3 predicted_position;
	float pad_1;
	glm::vec3 velocity;
	float pad_2;
	float density;
	float pad_3;
	float pad_4;
	float pad_5;
};
