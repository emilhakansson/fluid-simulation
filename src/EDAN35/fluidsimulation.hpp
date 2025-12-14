#pragma once

#include "core/InputHandler.h"
#include "core/FPSCamera.h"
#include "core/WindowManager.hpp"


class Window;


namespace edan35
{
	//! \brief Wrapper class for fluid sim
	class FluidSimulation {
	public:
		//! \brief Default constructor.
		//!
		//! It will initialise various modules of bonobo and retrieve a
		//! window to draw to.
		FluidSimulation(WindowManager& windowManager);

		//! \brief Default destructor.
		//!
		//! It will release the bonobo modules initialised by the
		//! constructor, as well as the window.
		~FluidSimulation();

		//! \brief Contains the logic of the assignment, along with the
		//! render loop.
		void run();

		static float calculateDensity(glm::vec3 samplePoint);

		static glm::vec3 calculatePressureForce(int particle_index);

		static float calculateSharedPressure(float densityA, float densityB);

		static float convertDensityToPressure(float density);

		static float smoothingKernel(float radius, float distance);

		static float smoothingKernelDerivative(float radius, float distance);

		static glm::vec3 randomDirection();

	private:
		FPSCameraf     mCamera;
		InputHandler   inputHandler;
		WindowManager& mWindowManager;
		GLFWwindow* window;
	};
}
