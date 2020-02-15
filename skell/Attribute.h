#pragma once
#include <GL/glew.h>
#include <string>

struct Attribute {
	//const GLchar* name;
	std::string name;
	const GLuint index;
	const GLsizei num_elements;
};

struct BufferDef {
	const GLint index;
	const GLsizei num_elements;
};