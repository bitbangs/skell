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
				vert_shader_file >> line; //we should see if we get a precision, although we're not using this yet in any shaders so for now this is really something like "vec3" or "mat4" in line after this executes
				GLsizei num_elements = (GLsizei)(line.back() - 48); //because ascii '0' is 48 decimal
				vert_shader_file >> line; //get name
				line.pop_back(); //remove ';'
				//attributes.push_back({ line.c_str(), index++, num_elements});
				attributes.push_back({ line, index++, num_elements });
			}
			vert_shader_file >> line;
		}
	}

	//this seems kind of clunky...can we make this a generator and just return strings?
	std::vector<Attribute> GetAttributes() const {
		return attributes;
	}
};

