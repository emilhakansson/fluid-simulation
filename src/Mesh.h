#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

using glm::vec3;

class Mesh
{
public:
	Mesh(const std::vector<vec3>& vertices, const std::vector<unsigned int>& indices, GLenum drawMode = GL_TRIANGLES) : 
		vertices(vertices), 
		indices(indices), 
		drawMode{drawMode}
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &IBO);

		glBindVertexArray(VAO);

		// Copy the vertex data into VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), vertices.data(), GL_STATIC_DRAW);

		// Copy the index data into IBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		// Set the vertex attribute pointers
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
		glEnableVertexAttribArray(0);
	}

	static Mesh triangle(vec3 v1, vec3 v2, vec3 v3)
	{
		return Mesh
		(
			std::vector<vec3>{ v1, v2, v3 },
			std::vector<unsigned int>{ 0, 1, 2 }
		);
	}

	static Mesh quad(float size = 1.0f)
	{
		auto vertices = std::vector<vec3>
		{
			vec3(-size / 2,  size / 2, 0.0f),
			vec3( size / 2,  size / 2, 0.0f),
			vec3(-size / 2, -size / 2, 0.0f),
			vec3( size / 2, -size / 2, 0.0f)
		};

		auto indices = std::vector<unsigned int>
		{
			0, 1, 2,
			1, 3, 2
		};
		return Mesh(vertices, indices);
	}

	static Mesh cube(float width, float length, float height)
	{
		float w = width;
		float l = length;
		float h = height;

		auto vertices = std::vector<vec3>
		{
			// Bottom vertices
			vec3(0, 0, 0),
			vec3(w, 0, 0),
			vec3(0, h, 0),
			vec3(w, h, 0),

			// Top vertices
			vec3(0, 0, l),
			vec3(w, 0, l),
			vec3(0, h, l),
			vec3(w, h, l),
		};

		auto indices = std::vector<unsigned int>
		{
			// Front face
			4, 5, 6,
			5, 7, 6,

			// Right face
			5, 1, 7,
			1, 3, 7,

			// Back face
			1, 0, 3,
			0, 2, 3,

			// Left face
			0, 4, 2,
			4, 6, 2,

			// Bottom face
			0, 1, 4,
			1, 5, 4,

			// Top face
			6, 7, 2,
			7, 3, 2
		};

		return Mesh(vertices, indices);
	}

	static Mesh cubeWireframe(float width = 1.0f, float length = 1.0f, float height = 1.0f)
	{
		auto vertices = std::vector<vec3>
		{
			// Bottom vertices
			vec3(-width / 2, -length / 2, -height / 2),
			vec3( width / 2,     -length / 2, -height / 2),
			vec3(-width / 2,  length / 2, -height / 2),
			vec3( width / 2,  length / 2, -height / 2),

			// Top vertices
			vec3(-width / 2, -length / 2, height / 2),
			vec3( width / 2, -length / 2, height / 2),
			vec3(-width / 2,  length / 2, height / 2),
			vec3( width / 2,  length / 2, height / 2),
		};

		auto indices = std::vector<unsigned int>
		{
			// Bottom horizontal lines
			0, 1,
			0, 2,
			1, 3,
			2, 3,

			// Vertical lines
			0, 4,
			1, 5,
			2, 6,
			3, 7,

			// Top horizontal lines
			4, 5,
			4, 6,
			5, 7,
			6, 7
		};


		return Mesh(vertices, indices, GL_LINES);
	}

	void draw() const
	{
		glBindVertexArray(VAO);
		glDrawElements(drawMode, indices.size(), GL_UNSIGNED_INT, nullptr);
	}
	
	void drawInstances(unsigned int nbrInstances)
	{
		glBindVertexArray(VAO);
		glDrawElementsInstanced(drawMode, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr, nbrInstances);
	}

private:
	GLuint VAO;
	GLuint VBO;
	GLuint IBO;
	std::vector<vec3> vertices;
	std::vector<unsigned int>  indices;
	GLenum drawMode;
};

#endif