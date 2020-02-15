#pragma once
#include <GL/glew.h>
#include <LinearAlgebra/Vector.hpp>
#include <LinearAlgebra/Matrix.hpp>
#include "ShaderProgram.h"

class Drawer
{
private:
	ShaderProgram shader_program;

public:
	Drawer() = delete;
	Drawer(ShaderProgram shader_program, GLfloat aspect_ratio) :
		shader_program(shader_program)
	{
		shader_program.Use();

		//view matrix for the eye
		LinearAlgebra::Matrix<GLfloat> view(4, 4, {
			+1.0f, +0.0f, +0.0f, +0.0f,
			+0.0f, +1.0f, +0.0f, +0.0f,
			+0.0f, +0.0f, +1.0f, +0.0f,
			+0.0f, +0.0f, +0.5f, +1.0f //our position
		});
		shader_program.SetMatrixBuffer("view", view.GetPointerToData());

		//projection matrix for the eye
		LinearAlgebra::Matrix<GLfloat> projection(4, 4, {
			+1.0f / (+aspect_ratio * std::tanf(3.14159f / 4.0f)), +0.0f, +0.0f, +0.0f,
			0.0f, +1.0f / std::tanf(3.14159f / 4.0f), +0.0f, +0.0f,
			+0.0f, +0.0f, (1.0f - 100.0f) / (1.0f - 100.0f), +1.0f,
			+0.0f, +0.0f, (+2.0f * 100.0f * 1.0f) / (1.0f - 100.0f), +0.0f
		});
		shader_program.SetMatrixBuffer("projection", projection.GetPointerToData());

		//set the ambient light
		GLfloat ambient = 0.6f;
		shader_program.SetVectorBuffer("ambient", ambient, ambient, ambient, 1.0f);
	}

	void Draw(const GLfloat* model_data, GLsizei num_indices) {
		shader_program.Use(); //wasteful
		shader_program.SetMatrixBuffer("model", model_data); //set player model back
		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0); //draw the main square mesh
	}
};

