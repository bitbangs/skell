#pragma once
#include <algorithm>
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
	std::vector<std::string> attributes;

public:
	VertexShader(const GLchar* src) :
		Shader(src, glCreateShader(GL_VERTEX_SHADER))
	{
		//get what is needed from vertex shader src or compiled shader ??
		std::stringstream vert_shader_file(src);
		//if (vert_shader_file.is_open()) {
			std::string line;
			vert_shader_file >> line;
			while (line != "void") { //until start of main() signature
				if (line == "in") {
					vert_shader_file >> line; //get precision
					vert_shader_file >> line; //get name
					attributes.push_back(line); //need to remove ";"
				}
				vert_shader_file >> line;
			}
		//	vert_shader_file.close();
		//}
	}

	//this seems kind of clunky...can we make this a generator and just return strings?
	std::vector<std::string> GetAttributes() const {
		return attributes;
	}
};

