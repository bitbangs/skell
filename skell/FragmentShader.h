#pragma once
#include <GL/glew.h>
#include "Shader.h"
class FragmentShader :
	public Shader
{
private:

public:
	FragmentShader(const GLchar* src) :
		Shader(src, glCreateShader(GL_FRAGMENT_SHADER))
	{
		//get what is needed for fragment shaders from src or compiled shader ??
	}
};

