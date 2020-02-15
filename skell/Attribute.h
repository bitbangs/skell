#pragma once
#include <GL/glew.h>

struct Attribute {
	const GLchar* name;
	const GLuint index;
	const GLsizei num_elements;
};