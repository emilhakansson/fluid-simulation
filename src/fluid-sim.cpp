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
#include "Profiler.h"

#include <iostream>
#include <vector>
#include <omp.h>


namespace chrono = std::chrono;

constexpr int PHYSICS_UPDATE_RATE{ 60 }; // in Hertz
constexpr float PHYSICS_UPDATE_TIMESTEP{ 1.0f / PHYSICS_UPDATE_RATE }; // in seconds
constexpr unsigned int grid_size_x = 150u;
constexpr unsigned int grid_size_y = 150u;
constexpr unsigned int grid_size_z = 150u;
constexpr unsigned int nbrParticles = grid_size_x * grid_size_y * grid_size_z;

int main()
{
	float elapsed = 0.0f;
	float lag = 0.0f;
	SimpleWindow window = SimpleWindow(1600, 900, "Hello window");
	InputHandler input = InputHandler(window);
	Camera camera = Camera(60.0f, 16.0f/9.0f, 0.01f, 1000.0f);
	
	ShaderProgram particleShader = ShaderProgram({ { ShaderType::Vertex, "particle.vert" }, { ShaderType::Fragment, "particle.frag" } });
	particleShader.setUniforms([&camera](GLuint program)
		{
			glUniform3fv(glGetUniformLocation(program, "cameraRight"), 1, glm::value_ptr(camera.getRight()));
			glUniform3fv(glGetUniformLocation(program, "cameraForward"), 1, glm::value_ptr(camera.getForward()));
		});
	
	ShaderProgram computeShader = ShaderProgram({ { ShaderType::Compute, "simulation.comp" } });
	

	std::vector<Particle> particles;
	particles.reserve(nbrParticles);

	Graphics particleGraphics = Graphics(Mesh::quad(1.0f), particleShader);

	#pragma omp parallel for
	for (unsigned int i = 0; i < grid_size_x; i++)
	{
		for (unsigned int j = 0; j < grid_size_y; j++)
		{
			for (unsigned int k = 0; k < grid_size_z; k++)
			{
				Particle p;
				p.position = vec3(i * 2.0f, j * 2.0f, k * 2.0f);
				particles.emplace_back(p);
			}
		}
	}

	GLuint particle_ssbo;
	glGenBuffers(1, &particle_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, particle_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particle_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle) * nbrParticles, particles.data(), GL_DYNAMIC_DRAW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	auto lastTime = chrono::high_resolution_clock::now();

	while (!window.shouldClose())
	{
		auto nowTime = chrono::high_resolution_clock::now();
		float deltaTime = chrono::duration<float>(nowTime - lastTime).count();
		lastTime = nowTime;
		elapsed += deltaTime;
		lag += deltaTime;

		// --- Input ---
		camera.update(deltaTime, input);

		// --- Physics ---
		int updates = static_cast<int>(lag / PHYSICS_UPDATE_TIMESTEP);
		for (int i = 0; i < updates; i++)
		{
		//	for (Particle &p : particles)
		//	{
		//		p.update(PHYSICS_UPDATE_TIMESTEP);
		//	}

		}
		lag -= PHYSICS_UPDATE_TIMESTEP * updates;

		// --- Render ---
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		particleGraphics.render(camera.getViewProjection(), nbrParticles);

		// --- Draw UI ---
		bool opened = window.beginUI("Controls");
		if (opened)
		{
			int mins = static_cast<int>(elapsed) / 60;
			int seconds = static_cast<int>(elapsed) % 60;
			ImGui::BeginGroup();
			ImGui::Text("Elapsed time: %02d:%02d", mins, seconds);
			ImGui::Text("Framerate: %.0f", ImGui::GetIO().Framerate);
			ImGui::EndGroup();
		}
		window.endUI();
	}
	return 0;
}