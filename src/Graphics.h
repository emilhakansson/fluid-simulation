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

private:
	Mesh mesh;
	Shader shader;

	vec3 scale{ vec3(1.0f) };
	vec3 translation{ vec3(0.0f) };
	mat3 rotation{ mat3(1.0f) };

	mat4 modelMatrix() const
	{
		return glm::mat4
		   (      scale.x,             0,             0, 0,
			            0,       scale.y,             0, 0,
			            0,             0,       scale.z, 0,
			translation.x, translation.y, translation.z, 1);
	}
};

#endif