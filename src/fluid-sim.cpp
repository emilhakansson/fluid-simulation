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
#include "fluid-sim.h"

#include <iostream>
#include <vector>
#include <random>
#include <omp.h>

namespace chrono = std::chrono;
using glm::vec3;

constexpr int PHYSICS_UPDATE_RATE{ 60 }; // in Hertz
constexpr float PHYSICS_UPDATE_TIMESTEP{ 1.0f / PHYSICS_UPDATE_RATE }; // in seconds
constexpr int nbrInvocations = 64;
constexpr int maxParticles = 50000;
vec3 bounds = vec3(25.0f, 50.0f, 35.0f);

static int nbrWorkGroups(int nbrParticles)
{
	return std::ceil(static_cast<float>(nbrParticles / static_cast<float>(nbrInvocations)));
}

int main()
{
	int nbrParticles = 8000;
	int prevNbrParticles = nbrParticles;
	float spacing = 0.0f;
	float prevSpacing = spacing;

	// --- Timing ---
	float deltaTime = 0.0f;
	float elapsed = 0.0f;
	float lag = 0.0f;
	bool started = false;
	bool paused = false;

	// --- Physics constants ---
	float gravity = 9.8f;
	float collisionDamping = 0.1f;
	float targetDensity = 1.25f;
	float influenceRadius = 2.0f;
	float pressureMultiplier = 300.0f;
	float particleScale = 1.0f;

	// Box controls
	bool hideBox = false;
	float rotationX = 0.0f;
	float rotationY = 0.0f;
	float rotationZ = 0.0f;

	SimpleWindow window = SimpleWindow(1600, 900, "Hello window");
	InputHandler input = InputHandler(window);
	Camera camera = Camera(60.0f, 16.0f/9.0f, 0.01f, 1000.0f);
	camera.setTranslation(vec3(5.0f, 0.0f, -15.f));

	Shader defaultShader = Shader("default.vert", "default.frag");
	Shader particleShader = Shader("particle.vert", "particle.frag");
	particleShader.setUniforms([&camera, &targetDensity, &particleScale, &spacing](GLuint program)
		{
			glUniform3fv(glGetUniformLocation(program, "cameraRight"), 1, glm::value_ptr(camera.getRight()));
			glUniform3fv(glGetUniformLocation(program, "cameraForward"), 1, glm::value_ptr(camera.getForward()));
			glUniform1f(glGetUniformLocation(program, "targetDensity"), targetDensity);
			glUniform1f(glGetUniformLocation(program, "particleScale"), particleScale);
			glUniform1f(glGetUniformLocation(program, "spacing"), spacing);
		});
	
	Graphics particleGraphics = Graphics(Mesh::quad(), particleShader);
	Graphics box = Graphics(Mesh::cubeWireframe(), defaultShader);

	Shader computeShader = Shader("simulation.comp");
	computeShader.setUniforms(
		[&nbrParticles, &deltaTime, &paused, &box, &gravity, &collisionDamping, 
		&targetDensity, &influenceRadius, &pressureMultiplier]
		(GLuint program)
		{
			glUniform1i(glGetUniformLocation(program, "nbrParticles"), nbrParticles);
			glUniform1f(glGetUniformLocation(program, "deltaTime"), deltaTime);
			glUniform1i(glGetUniformLocation(program, "paused"), paused);
			glUniform1f(glGetUniformLocation(program, "gravity"), gravity);
			glUniform1f(glGetUniformLocation(program, "collisionDamping"), collisionDamping);
			glUniform3fv(glGetUniformLocation(program, "bounds"), 1, glm::value_ptr(bounds));
			glUniformMatrix4fv(glGetUniformLocation(program, "worldToLocal"), 1, false, glm::value_ptr(box.modelMatrix()));
			glUniformMatrix4fv(glGetUniformLocation(program, "localToWorld"), 1, false, glm::value_ptr(glm::inverse(box.modelMatrix())));
			glUniform1f(glGetUniformLocation(program, "targetDensity"), targetDensity);
			glUniform1f(glGetUniformLocation(program, "influenceRadius"), influenceRadius);
			glUniform1f(glGetUniformLocation(program, "pressureMultiplier"), pressureMultiplier);
		});
	
	std::vector<Particle> particles = initializeParticles(nbrParticles, spacing);
	GLuint particle_ssbo = createSSBO(5, particles, maxParticles);

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
		bool particlesChanged = (nbrParticles != prevNbrParticles || spacing != prevSpacing);
		if (!started && particlesChanged)
		{
			nbrParticles = std::min(nbrParticles, maxParticles);
			particles = initializeParticles(nbrParticles, spacing);
			uploadToSSBO(5, particles, maxParticles);
		}
		prevNbrParticles = nbrParticles;
		prevSpacing = spacing;
		if (started && !paused)
		{
			dispatchCompute(computeShader, nbrParticles);
		}

		// --- Render ---
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		particleGraphics.render(camera.getViewProjection(), nbrParticles);
		if (!hideBox) box.render(camera.getViewProjection());
		box.setScale(bounds);
		box.setRotationX(rotationX);
		box.setRotationY(rotationY);
		box.setRotationZ(rotationZ);

		// --- Draw UI ---
		bool opened = window.beginUI("Controls");
		if (opened)
		{
			int mins = static_cast<int>(elapsed) / 60;
			int seconds = static_cast<int>(elapsed) % 60;
			ImGui::BeginGroup();
			ImGui::Text("Elapsed time: %02d:%02d", mins, seconds);
			ImGui::Text("Framerate: %.0f", ImGui::GetIO().Framerate);
			if (ImGui::Button(started ? "Reset" : "Start"))
			{
				if (started)
				{
					started = false;
					particles = initializeParticles(nbrParticles, spacing);
					uploadToSSBO(5, particles, maxParticles);
				}
				else
				{
					started = true;
				}
			};
			if (started)
			{
				ImGui::Checkbox("Pause", &paused);
			}
			
			ImGui::Separator();
			
			ImGui::Text("Box bounds");
			ImGui::Checkbox("Hide", &hideBox);
			ImGui::SliderFloat("Width", &bounds.x, 10.0f, 100.0f);
			ImGui::SliderFloat("Height", &bounds.y, 10.0f, 100.0f);
			ImGui::SliderFloat("Length", &bounds.z, 10.0f, 100.0f);
			ImGui::Text("Box rotation");
			ImGui::SliderFloat("X", &rotationX, 0.0f, 6.28f);
			ImGui::SliderFloat("Y", &rotationY, 0.0f, 6.28f);
			ImGui::SliderFloat("Z", &rotationZ, 0.0f, 6.28f);

			ImGui::Separator();
			
			ImGui::Text("Constants");
			ImGui::SliderInt("Particles", &nbrParticles, 1, 13824);
			ImGui::SliderFloat("Spacing", &spacing, 0.0f, 1.0f);
			ImGui::SliderFloat("Gravity", &gravity, -10.0f, 10.0f);
			ImGui::SliderFloat("Collision damping", &collisionDamping, 0.0f, 1.0f);
			ImGui::SliderFloat("Particle scale", &particleScale, 0.0f, 2.0f);
			ImGui::SliderFloat("Target density", &targetDensity, 1.0f, 5.0f);
			ImGui::SliderFloat("Pressure multiplier", &pressureMultiplier, 1.0f, 300.0f);
			ImGui::SliderFloat("Influence radius", &influenceRadius, 1.0f, 10.0f);
			ImGui::EndGroup();
		}
		window.endUI();
	}
	return 0;
}

void dispatchCompute(Shader& computeShader, int nbrParticles)
{
	computeShader.dispatch(nbrWorkGroups(nbrParticles), 1, 1, Compute::ResolveCollisions);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	computeShader.dispatch(nbrWorkGroups(nbrParticles), 1, 1, Compute::Density);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	computeShader.dispatch(nbrWorkGroups(nbrParticles), 1, 1, Compute::Pressure);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	computeShader.dispatch(nbrWorkGroups(nbrParticles), 1, 1, Compute::Position);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

static std::vector<Particle> initializeParticles(int nbrParticles, float spacing)
{
	std::vector<Particle> particles;
	particles.resize(maxParticles);
	int gridSize = std::ceil(std::cbrt(nbrParticles));
	int halfSize = gridSize / 2;
	float offset = 1 + spacing;

	#pragma omp parallel for
	for (int x = 0; x < gridSize; x++)
	{
		for (int y = 0; y < gridSize; y++)
		{
			for (int z = 0; z < gridSize; z++)
			{
				int index = x * gridSize * gridSize + y * gridSize + z;
				if (index >= nbrParticles) break;
				Particle p;
				p.position = vec3((x - halfSize) * offset, 
								  (y - halfSize) * offset,
								  (z - halfSize) * offset);
				p.velocity = vec3(0.0f);
				p.density = 1.0f;
				particles[index] = p;
				index++;
			}
		}
	}
	return particles;
}