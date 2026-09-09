#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <GLFW/glfw3.h>
#include "SimpleWindow.h"
#include <iostream>

class InputHandler
{
public:
	InputHandler(const SimpleWindow& window) : window(window) {};

	bool InputHandler::left() const
	{
		return window.keyPressed(LEFT);
	}

	bool InputHandler::right() const
	{
		return window.keyPressed(RIGHT);
	}

	bool InputHandler::forward() const
	{
		return window.keyPressed(FORWARD);
	}

	bool InputHandler::backward() const
	{
		return window.keyPressed(BACKWARD);
	}

	bool InputHandler::up() const
	{
		return window.keyPressed(UP);
	}

	bool InputHandler::down() const
	{
		return window.keyPressed(DOWN);
	}

	bool InputHandler::rotateLeft() const
	{
		return window.keyPressed(ROTATE_LEFT);
	}

	bool InputHandler::rotateRight() const
	{
		return window.keyPressed(ROTATE_RIGHT);
	}
	bool InputHandler::sprint() const
	{
		return window.keyPressed(SPRINT);
	}

private:
	SimpleWindow window;

	static const int LEFT{ GLFW_KEY_A };
	static const int RIGHT{ GLFW_KEY_D };
	static const int FORWARD{ GLFW_KEY_W };
	static const int BACKWARD{ GLFW_KEY_S };
	static const int UP{ GLFW_KEY_SPACE };
	static const int DOWN{ GLFW_KEY_LEFT_CONTROL };
	static const int ROTATE_LEFT{ GLFW_KEY_Q };
	static const int ROTATE_RIGHT{ GLFW_KEY_E };
	static const int SPRINT{ GLFW_KEY_LEFT_SHIFT };
};

#endif