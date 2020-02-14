#pragma once
#include <initializer_list>
#include <vector>
#include "VertexShader.h"
#include "FragmentShader.h"

class ShaderProgram
{
private:

public:
	ShaderProgram() = delete;
	ShaderProgram(VertexShader vert_shader, FragmentShader frag_shader);
};

