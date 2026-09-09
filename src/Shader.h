#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <variant>

#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using std::string;

enum class ShaderType { Vertex, Fragment, Compute };

using ShaderData = std::map<ShaderType, const char*>;
static const string SHADER_PATH{ "../../../shaders/" };

class ShaderProgram
{
public:
	ShaderProgram(const ShaderData& data) : _setUniforms{ [](GLenum program) {} }
	{
		shaderProgram = glCreateProgram();
		for (const auto& [type, filename] : data)
		{
			GLuint shader = createShader(type, filename);
			glAttachShader(shaderProgram, shader);
			glDeleteShader(shader);
		}
		link(shaderProgram);
	}

	void use()
	{
		glUseProgram(shaderProgram);
		_setUniforms(shaderProgram);
	}

	void setUniforms(const std::function<void(GLuint)>& setUniforms)
	{
		_setUniforms = setUniforms;
	}

	GLuint getId()
	{
		return shaderProgram;
	}

private:
	GLuint shaderProgram;
	std::function<void(GLuint)> _setUniforms;

	string getShaderSource(const char* filename)
	{
		std::stringstream buffer;
		std::ifstream is;
		is.open(SHADER_PATH + filename);
		buffer << is.rdbuf();
		is.close();
		return buffer.str();
	}

	void compile(GLuint shader, ShaderType type)
	{
		glCompileShader(shader);
		int compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			char infoLog[512];
			glGetShaderInfoLog(shader, 512, NULL, infoLog);
			std::cout << "ERROR: " << (type == ShaderType::Vertex ? "Vertex" : "Fragment") << " shader compilation failed." << std::endl
				<< infoLog << std::endl;
		}
	}

	void link(GLuint program)
	{
		glLinkProgram(shaderProgram);
		int linked;
		glGetProgramiv(program, GL_LINK_STATUS, &linked);
		if (!linked)
		{
			char infoLog[512];
			glGetProgramInfoLog(program, 512, NULL, infoLog);
			std::cout << "ERROR: Shader program linking failed." << std::endl
				<< infoLog << std::endl;
		}
	}

	GLuint createShader(ShaderType type, const char* filename)
	{
		string source = getShaderSource(filename);
		const char* src = source.c_str();
		GLuint shader = [&]()
			{
				switch (type)
				{
				case ShaderType::Vertex: return glCreateShader(GL_VERTEX_SHADER);
				case ShaderType::Fragment: return glCreateShader(GL_FRAGMENT_SHADER);
				case ShaderType::Compute: return glCreateShader(GL_COMPUTE_SHADER);
				}
			}();

		glShaderSource(shader, 1, &src, NULL);
		compile(shader, type);
		return shader;
	}
};

#endif