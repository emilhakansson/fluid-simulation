#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "Mesh.h"
#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using glm::vec3;
using glm::vec4;
using glm::mat4;


class Graphics
{
public:
	Graphics(Mesh mesh, ShaderProgram shaderProgram) : 
		mesh{ mesh },
		shaderProgram{ shaderProgram }
	{};

	void render(mat4 viewProjection, unsigned int nbrInstances = 1u)
	{
		assert(nbrInstances > 0u && "Number of instances must be non-zero");
		shaderProgram.use();
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram.getId(), "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram.getId(), "viewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));
		nbrInstances < 2u ? mesh.draw() : mesh.drawInstances(nbrInstances);
	}

	void update(vec3 position)
	{
		model[3] = vec4(position, 1.0f);
	}

	mat4 getModel() const { return model; }

private:
	Mesh mesh;
	ShaderProgram shaderProgram;
	mat4 model{ mat4(1.0f) };
};

#endif