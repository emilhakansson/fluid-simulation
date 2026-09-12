#ifndef PARTICLE_H
#define PARTICLE_H

#include "glm/glm.hpp"

#include <iostream>

using glm::vec3;

struct Particle
{
	vec3 position;
	float pad1;
	vec3 velocity;
	float pad2;
};

#endif