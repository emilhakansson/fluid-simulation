#include "fluidsimulation.hpp"
#include "EDAF80/parametric_shapes.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/node.hpp"
#include "core/ShaderProgramManager.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <tinyfiledialogs.h>

#include <clocale>
#include <stdexcept>
#include "neighborhood_search.cpp"
#include "Particle.h"

edan35::FluidSimulation::FluidSimulation(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),
		static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
		0.01f, 1000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0 };

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 4", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}

	bonobo::init();
}

edan35::FluidSimulation::~FluidSimulation()
{
	bonobo::deinit();
}

float gravity = 9.80f;
float mass = 1.0f;

int particles_nb = 30000;

// Without further optimization, 40000 particles seems to be the limit of what my GPU can handle while retaining 60fps.
int max_particle_count = 40000;

float particle_influence_radius = 1.5f;
float target_density = 1.0f;
float pressure_multiplier = 750.0f;
float near_pressure_multiplier = 2.0f;
float collision_dampening = 0.7f;

float bounds_x = 80.0f;
float bounds_y = 100.0f;
float bounds_z = 40.0f;

float deltaTime;

bool pause_animation = true;

const glm::vec3 down = glm::vec3(0.0f, -1.0f, 0.0f);

std::vector<Particle> particles = std::vector<Particle>(max_particle_count);
std::vector<glm::vec3> positions = std::vector<glm::vec3>(particles_nb);
std::vector<glm::vec3> predicted_positions = std::vector<glm::vec3>(particles_nb);
std::vector<glm::vec3> velocities = std::vector<glm::vec3>(particles_nb);
std::vector<float> densities = std::vector<float>(particles_nb);
std::vector<glm::vec3> pressure_forces = std::vector<glm::vec3>(particles_nb);
edan35::NeighborhoodSearch neighborhood_search = edan35::NeighborhoodSearch::NeighborhoodSearch(predicted_positions, particle_influence_radius);

static void setComputeUniforms(GLuint program) {
	glUniform1i(glGetUniformLocation(program, "particles_nb"), particles_nb);
	glUniform1f(glGetUniformLocation(program, "gravity"), gravity);
	glUniform1f(glGetUniformLocation(program, "deltaTime"), deltaTime);
	glUniform1f(glGetUniformLocation(program, "collision_damping"), collision_dampening);
	glUniform1f(glGetUniformLocation(program, "min_x"), -bounds_x / 2);
	glUniform1f(glGetUniformLocation(program, "max_x"),  bounds_x / 2);
	glUniform1f(glGetUniformLocation(program, "min_y"), -bounds_y / 2);
	glUniform1f(glGetUniformLocation(program, "max_y"),  bounds_y / 2);
	glUniform1f(glGetUniformLocation(program, "min_z"), -bounds_z / 2);
	glUniform1f(glGetUniformLocation(program, "max_z"),  bounds_z / 2);
	glUniform1f(glGetUniformLocation(program, "influence_radius"), particle_influence_radius);
	glUniform1f(glGetUniformLocation(program, "target_density"), target_density);
	glUniform1f(glGetUniformLocation(program, "pressure_multiplier"), pressure_multiplier);
	glUniform1f(glGetUniformLocation(program, "near_pressure_multiplier"), near_pressure_multiplier);
}

static void initializeParticlePositions(int particles_nb)
{
	int grid_size = 1;
	while (grid_size * grid_size * grid_size < particles_nb) {
		grid_size++;
	}
	int idx = 0;
	for (int i = 0; i < grid_size; i++) {
		for (int j = 0; j < grid_size; j++) {
			for (int k = 0; k < grid_size; k++) {
				if (idx >= particles_nb) {
					break;
				}
				float x = -bounds_x / 2 + i;
				float y = -bounds_y / 2 + j;
				float z = -bounds_z / 2 + k;
				glm::vec3 position = glm::vec3(x, y, z);
				particles[idx].position = position;
				particles[idx].predicted_position = position;
				idx++;
				
			}
		}
	}
}

static void initializeParticles(int particles_nb, float gap) {
	for (int i = 0; i < particles_nb; i++) {
		particles[i] = Particle();
	}
	initializeParticlePositions(particles_nb);
}



void
edan35::FluidSimulation::run() {
	assert(particles_nb <= max_particle_count);
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 100.0f, 100.0f));
	mCamera.mWorld.LookAt(glm::vec3(0.0f));
	mCamera.mMouseSensitivity = glm::vec2(0.001f);
	mCamera.mMovementSpeed = glm::vec3(30.0f);

	glm::vec3 camera_position = mCamera.mWorld.GetTranslation();
	bool shader_reload_failed = false;

	// Create the shader programs
	ShaderProgramManager program_manager;

	GLuint fallback_shader = 0u;
	program_manager.CreateAndRegisterProgram("Fallback",
		{ { ShaderType::vertex, "common/fallback.vert" },
		  { ShaderType::fragment, "common/fallback.frag" }
		},
		fallback_shader);
	if (fallback_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}

	GLuint particle_shader = 0u;
	program_manager.CreateAndRegisterProgram("Particle",
		{ { ShaderType::vertex, "EDAN35/particle.vert" },
		  { ShaderType::fragment, "EDAN35/particle.frag" }
		},
		particle_shader);
	if (particle_shader == 0u) {
		LogError("Failed to load particle shader");
		return;
	}

	glm::vec3 initial_position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 initial_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	float radius = 1.0f;

	initializeParticles(particles_nb, 10.0f);

	// Set up uniforms

	glm::vec3 camera_up = mCamera.mWorld.GetUp();
	glm::vec3 camera_right = mCamera.mWorld.GetRight();
	float particle_scale = 1.0f;
	bool velocity_shading = false;
	auto const set_uniforms = [&camera_up, &camera_right, &particle_scale, &velocity_shading](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "camera_up"), 1, glm::value_ptr(camera_up));
		glUniform3fv(glGetUniformLocation(program, "camera_right"), 1, glm::value_ptr(camera_right));
		glUniform1f(glGetUniformLocation(program, "particle_scale"), particle_scale);
		glUniform1i(glGetUniformLocation(program, "velocity_shading"), velocity_shading);
		};

	// Set up instanced quad mesh

	bonobo::mesh_data data = parametric_shapes::createInstancedQuad(particles_nb);
	Node particlenode;
	particlenode.set_geometry(data);
	particlenode.set_program(&particle_shader, set_uniforms);

	// Set up compute shader
	GLuint compute_shader = 0u;
	program_manager.CreateAndRegisterComputeProgram("Compute", "EDAN35/fluid_sim.comp", compute_shader);
	if (compute_shader == 0u) {
		LogError("Failed to load compute shader");
		return;
	}

	GLuint particle_ssbo;
	glGenBuffers(1, &particle_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, particle_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particle_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle) * max_particle_count, particles.data(), GL_DYNAMIC_DRAW);

	glEnable(GL_DEPTH_TEST);

	auto lastTime = std::chrono::high_resolution_clock::now();
	int particles_nb_prev = particles_nb;

	while (!glfwWindowShouldClose(window)) {
		

		auto nowTime = std::chrono::high_resolution_clock::now();
		auto deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		deltaTime = static_cast<float>(deltaTimeUs.count()) / 1000000.0f;
		lastTime = nowTime;

		// Since the shader storage buffer size must be static, we have to 
		// check if the number of particles exceeds the max count. Then we 
		// spawn work groups based on the number of particles.
		bool particles_nb_changed = particles_nb != particles_nb_prev;
		particles_nb_prev = particles_nb;
		particles_nb = std::min(particles_nb, max_particle_count);
		int work_groups_nb = ceil(particles_nb / 128.0f);

		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();
		mCamera.Update(deltaTimeUs, inputHandler);
		camera_position = mCamera.mWorld.GetTranslation();
		camera_up = mCamera.mWorld.GetUp();
		camera_right = mCamera.mWorld.GetRight();

		if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			shader_reload_failed = !program_manager.ReloadAllPrograms();
			if (shader_reload_failed)
				tinyfd_notifyPopup("Shader Program Reload Error",
					"An error occurred while reloading shader programs; see the logs for details.\n"
					"Rendering is suspended until the issue is solved. Once fixed, just reload the shaders again.",
					"error");
		}

		int framebuffer_width, framebuffer_height;
		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
		glViewport(0, 0, framebuffer_width, framebuffer_height);
		mWindowManager.NewImGuiFrame();
		/** Handle inputs
			...
		*/
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		if (!shader_reload_failed) {
			if (!pause_animation) {
				glUseProgram(compute_shader);
				setComputeUniforms(compute_shader);
				glDispatchCompute(work_groups_nb, 1, 1);
				if (work_groups_nb > 1) {
					glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
				}
				
			}
			if (particles_nb_changed) {
				initializeParticlePositions(particles_nb);
				glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle) * max_particle_count, particles.data(), GL_DYNAMIC_DRAW);
			}
			particlenode.renderInstanced(particles_nb, mCamera.GetWorldToClipMatrix());
		}

		bool opened = ImGui::Begin("Scene Control", nullptr, ImGuiWindowFlags_None);
		if (opened) {
			ImGui::Text("Frame rate: %f", ImGui::GetIO().Framerate);
			ImGui::Checkbox("Pause animation", &pause_animation);
			ImGui::Checkbox("Velocity shading", &velocity_shading);

			ImGui::Separator();

			ImGui::SliderInt("Particle count", &particles_nb, 1, max_particle_count);
			ImGui::SliderFloat("Particle scale", &particle_scale, 0.1f, 2.0f);
			ImGui::SliderFloat("Influence radius", &particle_influence_radius, 0.1f, 3.0f);
			ImGui::SliderFloat("Target density", &target_density, 0.1f, 3.0f);
			ImGui::SliderFloat("Pressure multiplier", &pressure_multiplier, 1.0f, 1000.0f);
			ImGui::SliderFloat("Near pressure multiplier", &near_pressure_multiplier, 0.1f, 3.0f);
			ImGui::SliderFloat("Gravity", &gravity, -20.0f, 20.0f);
			ImGui::SliderFloat("Collision damping", &collision_dampening, 0.05f, 1.0f);
			
			ImGui::Separator();

			ImGui::SliderFloat("Box width", &bounds_x, 10.0f, 150.0f);
			ImGui::SliderFloat("Box height", &bounds_y, 10.0f, 100.0f);
			ImGui::SliderFloat("Box depth", &bounds_z, 10.0f, 100.0f);
		}
		ImGui::End();
		mWindowManager.RenderImGuiFrame(true);
		glfwSwapBuffers(window);
	}
	
}


float edan35::FluidSimulation::calculateDensity(glm::vec3 samplePoint) {
	float density = 0.0f;
	for (int i : neighborhood_search.getNeighborIndices(samplePoint)) {
		float influence = smoothingKernel(particle_influence_radius, glm::distance(samplePoint, predicted_positions[i]));
		density += influence * mass;
	}
	return density;
}

glm::vec3 edan35::FluidSimulation::calculatePressureForce(int particle_index)
{
	glm::vec3 pressure_force = glm::vec3(0);

	for (int other_index : neighborhood_search.getNeighborIndices(predicted_positions[particle_index])) {
		if (particle_index == other_index) continue;
		glm::vec3 offset = predicted_positions[other_index] - predicted_positions[particle_index];
		float distance = glm::length(offset);
		glm::vec3 dir = distance == 0 ? randomDirection() : offset / distance;
		float slope = smoothingKernelDerivative(particle_influence_radius, distance);
		float density = densities[particle_index];
		float sharedPressure = calculateSharedPressure(density, densities[other_index]);
		pressure_force += sharedPressure * dir * slope * mass / density;
	}
	return pressure_force;
}


float edan35::FluidSimulation::calculateSharedPressure(float densityA, float densityB) {
	float pressureA = convertDensityToPressure(densityA);
	float pressureB = convertDensityToPressure(densityB);
	return (pressureA + pressureB) / 2;
}

glm::vec3 edan35::FluidSimulation::randomDirection() {
	float x = (float)rand() / RAND_MAX;
	float y = (float)rand() / RAND_MAX;
	float z = (float)rand() / RAND_MAX;
	return glm::normalize(glm::vec3(x, y, z));
}

// The density of our fluid should be homogenous, so we apply pressure to each point to move it in the right direction.
// How much depends on how dense the region is. This approximation is not really how fluids behave in reality, but it works well enough.
float edan35::FluidSimulation::convertDensityToPressure(float density) {
	float densityError = density - target_density;
	float pressure = densityError * pressure_multiplier;
	return pressure;
}

// This is the "density contribution" from a particle with a given "influence" radius, at a given distance from the particle
// We use this to approximate a continuous density field from discrete particles.
float edan35::FluidSimulation::smoothingKernel(float radius, float distance) {
	if (distance >= radius) return 0;
	float volume = glm::pi<float>() * std::pow(radius, 4) / 6;
	float value = distance - radius;
	return value * value / volume;
}

// The gradient or slope of the density contribution, i.e. how fast the density is changing at a sample distance from a particle.
// We use this to approximate how much force the particle should be exerting on its neighbors.
float edan35::FluidSimulation::smoothingKernelDerivative(float radius, float distance) {
	if (distance >= radius) return 0;
	float scale = 12 / (std::pow(radius, 4) * glm::pi<float>());
	return (distance - radius) * scale;
}

int main()
{
	Bonobo framework;
	edan35::FluidSimulation fluid_simulation(framework.GetWindowManager());
	fluid_simulation.run();
}
