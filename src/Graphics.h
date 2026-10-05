#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "Mesh.h"
#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;


class Graphics
{
public:
	Graphics(Mesh mesh, Shader shader) : 
		mesh{ mesh },
		shader{ shader }
	{};

	void render(mat4 viewProjection, unsigned int nbrInstances = 1u)
	{
		assert(nbrInstances > 0u && "Number of instances must be non-zero");
		shader.use();
		glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "viewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));
		nbrInstances < 2u ? mesh.draw() : mesh.drawInstances(nbrInstances);
	}

	void setTranslation(vec3 position)
	{
		translation = position;
	}

	void setScale(vec3 _scale)
	{
		scale = _scale;
	}

	void setRotationX(float theta)
	{
		float sin_t = glm::sin(theta);
		float cos_t = glm::cos(theta);
		rotationX = mat3
		(
			1,      0,     0,
			0,  cos_t, sin_t,
			0, -sin_t, cos_t
		);
	}

	void setRotationY(float theta)
	{
		float sin_t = glm::sin(theta);
		float cos_t = glm::cos(theta);
		rotationY = mat3
		(
			 cos_t, 0, sin_t,
			     0, 1,     0,
			-sin_t, 0, cos_t
		);
	}

	void setRotationZ(float theta)
	{
		float sin_t = glm::sin(theta);
		float cos_t = glm::cos(theta);
		rotationZ = mat3
		(
			 cos_t, sin_t, 0,
			-sin_t, cos_t, 0,
			     0,     0, 1
		);
	}

private:
	Mesh mesh;
	Shader shader;

	vec3 scale{ vec3(1.0f) };
	vec3 translation{ vec3(0.0f) };
	mat3 rotationX{ mat3(1.0f) };
	mat3 rotationY{ mat3(1.0f) };
	mat3 rotationZ{ mat3(1.0f) };

	mat4 modelMatrix() const
	{
		mat4 rotation = glm::mat4(rotationZ * rotationY * rotationX);
		return rotation * glm::mat4
		   (      scale.x,             0,             0, 0,
			            0,       scale.y,             0, 0,
			            0,             0,       scale.z, 0,
			translation.x, translation.y, translation.z, 1);
	}
};

#endif