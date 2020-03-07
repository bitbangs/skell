#pragma once
#include <GL/glew.h>
#include "ShaderProgram.h"

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
class Drawer
{
private:
	ShaderProgram shader_program;

public:
	Drawer() = delete;
	Drawer(ShaderProgram shader, T aspect_ratio) :
		shader_program(shader)
	{
		shader_program.Use();
		//set the ambient light
		T ambient = 1.2f;
		shader_program.SetVectorBuffer("ambient", ambient, ambient, ambient, 1.0f);
		//set the light position for diffuse lighting
		shader_program.SetVectorBuffer("light_pos", +0.0f, +3.0f, -3.0f, +1.0f);
	}

	ShaderProgram GetShaderProgram() const {
		return shader_program;
	}
};
