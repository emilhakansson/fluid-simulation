#ifndef CAMERA_H
#define CAMERA_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "InputHandler.h"

using glm::vec3;
using glm::vec4;
using glm::mat4;

class Camera
{
public:
	Camera(float fovy, float aspectRatio, float near, float far): 
		projection{glm::perspective(glm::radians(fovy), aspectRatio, near, far)},
		view{mat4(1.0f)}
	{}

	mat4 getViewProjection()
	{
		return projection * view;
	}

	vec3 getPosition()
	{
		return view[3];
	}

	vec3 getForward()
	{
		vec3 forward = glm::normalize(view[2]);
		forward.z *= -1.0f;
		return forward;
	}

	vec3 getRight()
	{
		return glm::cross(getForward(), vec3(0.0f, 1.0f, 0.0f));
	}

	void update(float dt, const InputHandler& input)
	{
		vec3 dir = vec3(0.0f);
		float rotationY = 0.0f;

		if (input.forward())
		{
			dir += getForward();
		}
		if (input.backward())
		{
			dir -= getForward();
		}
		if (input.up())
		{
			dir.y += 1.0f;
		}
		if (input.down())
		{
			dir.y -= 1.0f;
		}
		if (input.left())
		{
			dir -= getRight();
		}
		if (input.right())
		{
			dir += getRight();
		}
		if (input.rotateLeft())
		{
			rotationY += dt;
		}
		if (input.rotateRight())
		{
			rotationY -= dt;
		}
		sprinting = input.sprint();

		mat4 rotation = glm::rotate(mat4(1.0f), -rotationY, vec3(0.0f, 1.0f, 0.0f));

		if (dir != vec3(0.0f))
		{
			dir = vec4(glm::normalize(dir), 0);
		}
		view = rotation * glm::translate(view, -dir * (sprinting ? sprintMultiplier : 1.0f) * moveSpeed * dt);
	}

	void setTranslation(vec3 position)
	{
		view[3] = vec4(position, 1);
	}

private:
	mat4 view;
	mat4 projection;
	float moveSpeed = 10.0f;
	float sprintMultiplier = 2.0f;
	bool sprinting = false;
};

#endif