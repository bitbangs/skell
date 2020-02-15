#pragma once
#include <algorithm>
#include "Attribute.h"
//#include <fstream>
#include <GL/glew.h>
#include "Shader.h"
#include <sstream>
#include <string>
#include <vector>

class VertexShader :
	public Shader
{
private:
	std::vector<Attribute> attributes;

public:
	VertexShader(const GLchar* src) :
		Shader(src, glCreateShader(GL_VERTEX_SHADER))
	{
		//get what is needed from vertex shader src or compiled shader ??
		std::stringstream vert_shader_file(src);
		std::string line;
		vert_shader_file >> line;
		GLuint index = 0;
		while (line != "void") { //until start of main() signature
			if (line == "in") {
				vert_shader_file >> line; //get precision
				GLsizei num_elements = (GLsizei)(line.back() - 48);
				vert_shader_file >> line; //get name
				line.pop_back(); //remove ';'
				attributes.push_back({ line.c_str(), index++, num_elements});
			}
			vert_shader_file >> line;
		}
	}

	//this seems kind of clunky...can we make this a generator and just return strings?
	std::vector<Attribute> GetAttributes() const {
		return attributes;
	}
};

