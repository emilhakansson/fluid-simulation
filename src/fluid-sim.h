#ifndef FLUIDSIM_H
#define FLUIDSIM_H

namespace Compute
{
	int ResolveCollisions = 0;
	int Density = 1;
	int Pressure = 2;
	int Position = 3;
};

static std::vector<Particle> initializeParticles(int nbrParticles);

static float random(float min, float max)
{
	return min + static_cast<float>(rand() / static_cast<float>(RAND_MAX / (max - min)));
}

template <typename T>
static GLuint createSSBO(int binding, const std::vector<T>& list, int capacity)
{
	GLuint SSBO;
	glGenBuffers(1, &SSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * capacity, list.data(), GL_DYNAMIC_DRAW);
	return SSBO;
}

template <typename T>
static GLuint createSSBO(int binding, const std::vector<T>& list)
{
	return createSSBO(binding, list, list.size());
}

#endif