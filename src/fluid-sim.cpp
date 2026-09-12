#include <chrono>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "shader.h"
#include "InputHandler.h"
#include "SimpleWindow.h"
#include "Particle.h"
#include "Graphics.h"
#include "Camera.h"

#include <iostream>
#include <vector>
#include <random>
#include <omp.h>


namespace chrono = std::chrono;
using glm::vec3;

constexpr int PHYSICS_UPDATE_RATE{ 60 }; // in Hertz
constexpr float PHYSICS_UPDATE_TIMESTEP{ 1.0f / PHYSICS_UPDATE_RATE }; // in seconds
constexpr unsigned int grid_size_x = 10u;
constexpr unsigned int grid_size_y = 100u;
constexpr unsigned int grid_size_z = 10u;
constexpr unsigned int nbrParticles = grid_size_x * grid_size_y * grid_size_z;
constexpr int nbrInvocations = 1;
const int nbrWorkGroups = std::ceil(static_cast<float>(nbrParticles / static_cast<float>(nbrInvocations)));


static float random(float min, float max)
{
	return min + static_cast<float>(rand() / static_cast<float>(RAND_MAX / (max - min)));
}

static std::vector<Particle> initializeParticles()
{
	std::vector<Particle> particles;
	particles.reserve(nbrParticles);

	#pragma omp parallel for
	for (int i = 0; i < grid_size_x; i++)
	{
		for (unsigned int j = 0; j < grid_size_y; j++)
		{
			for (unsigned int k = 0; k < grid_size_z; k++)
			{
				Particle p =
				{
					vec3(i * 2.0f, j * 2.0f, k * 2.0f), 0.0f,
					vec3(random(-5.0f, 5.0f), random(10.0f, 20.0f), random(-5.0f, 5.0f)), 0.0f
				};
				#pragma omp critical
				particles.push_back(p);
			}
		}
	}
	return particles;
}

static GLuint createSSBO(const std::vector<Particle> &particles, int binding)
{
	GLuint particleSSBO;
	glGenBuffers(1, &particleSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, particleSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle) * particles.size(), particles.data(), GL_DYNAMIC_DRAW);
	return particleSSBO;
}

int main()
{
	float deltaTime = 0.0f;
	float elapsed = 0.0f;
	float lag = 0.0f;
	float gravity = 9.8f;
	float collisionDamping = 0.1f;
	bool hideBox = false;
	vec3 bounds = vec3(15.0f, 15.0f, 15.0f);


	SimpleWindow window = SimpleWindow(1600, 900, "Hello window");
	InputHandler input = InputHandler(window);
	Camera camera = Camera(60.0f, 16.0f/9.0f, 0.01f, 1000.0f);
	camera.setTranslation(vec3(5.0f, 0.0f, -15.f));
	
	Shader defaultShader = Shader("default.vert", "default.frag");
	Shader particleShader = Shader("particle.vert", "particle.frag");
	particleShader.setUniforms([&camera](GLuint program)
		{
			glUniform3fv(glGetUniformLocation(program, "cameraRight"), 1, glm::value_ptr(camera.getRight()));
			glUniform3fv(glGetUniformLocation(program, "cameraForward"), 1, glm::value_ptr(camera.getForward()));
		});
	
	Shader computeShader = Shader("simulation.comp");
	computeShader.setUniforms([&gravity, &collisionDamping, &deltaTime, &bounds](GLuint program)
		{
			glUniform1f(glGetUniformLocation(program, "gravity"), gravity);
			glUniform1f(glGetUniformLocation(program, "collisionDamping"), collisionDamping);
			glUniform1f(glGetUniformLocation(program, "deltaTime"), deltaTime);
			glUniform1i(glGetUniformLocation(program, "nbrParticles"), nbrParticles);
			glUniform3fv(glGetUniformLocation(program, "bounds"), 1, glm::value_ptr(bounds));
		});
	
	Graphics particleGraphics = Graphics(Mesh::quad(), particleShader);
	Graphics box = Graphics(Mesh::cubeWireframe(), defaultShader);
	
	std::vector<Particle> particles = initializeParticles();

	GLuint particle_ssbo = createSSBO(particles, 5);

	auto lastTime = chrono::high_resolution_clock::now();

	while (!window.shouldClose())
	{
		auto nowTime = chrono::high_resolution_clock::now();
		deltaTime = chrono::duration<float>(nowTime - lastTime).count();
		lastTime = nowTime;
		elapsed += deltaTime;
		lag += deltaTime;

		// --- Input ---
		camera.update(deltaTime, input);

		// --- Physics ---
		int updates = static_cast<int>(lag * PHYSICS_UPDATE_RATE);
		for (int i = 0; i < updates; i++)
		{
		}
		computeShader.dispatch(nbrWorkGroups);
		lag -= PHYSICS_UPDATE_TIMESTEP * updates;

		// --- Render ---
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		particleGraphics.render(camera.getViewProjection(), nbrParticles);
		if (!hideBox) box.render(camera.getViewProjection());
		box.setScale(bounds);

		// --- Draw UI ---
		bool opened = window.beginUI("Controls");
		if (opened)
		{
			int mins = static_cast<int>(elapsed) / 60;
			int seconds = static_cast<int>(elapsed) % 60;
			ImGui::BeginGroup();
			ImGui::Text("Elapsed time: %02d:%02d", mins, seconds);
			ImGui::Text("Framerate: %.0f", ImGui::GetIO().Framerate);
			
			ImGui::Separator();
			
			ImGui::Text("Box bounds");
			ImGui::Checkbox("Hide", &hideBox);
			ImGui::SliderFloat("X", &bounds.x, 1.0f, 25.0f);
			ImGui::SliderFloat("Y", &bounds.y, 1.0f, 25.0f);
			ImGui::SliderFloat("Z", &bounds.z, 1.0f, 25.0f);

			ImGui::Separator();
			
			ImGui::Text("Constants");
			ImGui::SliderFloat("Gravity", &gravity, -10.0f, 10.0f);
			ImGui::SliderFloat("Collision damping", &collisionDamping, 0.0f, 1.0f);
			ImGui::EndGroup();
		}
		window.endUI();
	}
	return 0;
}