#ifndef SIMPLEWINDOW_H
#define SIMPLEWINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class SimpleWindow
{
public:
	SimpleWindow(int width, int height, const char* name)
	{
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(width, height, name, NULL, NULL);

		if (window == NULL)
		{
			glfwTerminate();
			throw std::exception("Failed to create GLFW window");
		}
		glfwMakeContextCurrent(window);
		//std::printf("OpenGl version: %s\n", glGetString(GL_VERSION));
		//std::printf("GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			throw std::exception("Failed to initialize GLAD");
		}
		glViewport(0, 0, width, height);

		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 430");
	}

	bool shouldClose() const
	{
		glfwPollEvents();
		glfwSwapBuffers(window);
		return glfwWindowShouldClose(window);
	}

	bool keyPressed(int key) const
	{
		return glfwGetKey(window, key) == GLFW_PRESS;
	}

	/* @brief Sets up a new ImGui frame for this window.
	 * @param name The name of the ImGui frame.
	 * @return true if successful, false otherwise.
	 */
	bool beginUI(const char* name)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		return ImGui::Begin(name, nullptr, ImGuiWindowFlags_None);
	}

	/* @brief Ends the current ImGui frame for this window. */
	void endUI()
	{
		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

private:
	GLFWwindow* window;
};

#endif