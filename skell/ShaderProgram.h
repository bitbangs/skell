#pragma once
#include <initializer_list>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <unordered_map>
#include <vector>
#include "Attribute.h"
#include "VertexShader.h"
#include "FragmentShader.h"

class ShaderProgram
{
private:
	GLuint id;
	std::shared_ptr<spdlog::logger> logger;
	std::unordered_map<std::string, BufferDef> buffers;

public:
	ShaderProgram() = delete;
	ShaderProgram(VertexShader& vert_shader, FragmentShader& frag_shader) :
		id(glCreateProgram())
	{
		//logger initialization
		std::string name = "shader_program" + std::to_string(id);
		try {
			logger = spdlog::basic_logger_mt(name + "_logger", name + "_log.txt");
		}
		catch (const spdlog::spdlog_ex & ex) {
			std::cout << "spdlog init failed: " << ex.what() << '\n';
		}

		//shader mapping
		glAttachShader(id, vert_shader.GetId());
		glAttachShader(id, frag_shader.GetId());
		GLuint attrib_id = 0;
		auto vert_attribs = vert_shader.GetAttributes();
		for (int ii = 0; ii < vert_attribs.size(); ++ii) {
			glBindAttribLocation(id, ii, vert_attribs[ii].name.c_str());
		}

		//linking
		glLinkProgram(id);
		GLint is_program_linked = GL_FALSE;
		glGetProgramiv(id, GL_LINK_STATUS, &is_program_linked);
		if (is_program_linked == GL_FALSE) {
			GLint err_msg_size = 0;
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &err_msg_size);

			GLchar* err_msg = new GLchar[err_msg_size];
			glGetProgramInfoLog(id, err_msg_size, NULL, err_msg);

			logger->warn("program did not link");
			if (err_msg == NULL) {
				logger->warn("could not get program link error message");
			}
			else {
				logger->warn(err_msg);
			}
			delete[] err_msg;
		}
		logger->info("program linked successfully");

		//combine attribute and uniform vectors into single map
		auto vert_unis = vert_shader.GetUniforms();
		for (auto ii : vert_unis) {
			buffers.insert({ii.name, {glGetUniformLocation(id, ii.name.c_str()), ii.num_elements }});
		}
		for (auto ii : vert_attribs) {
			buffers.insert({ ii.name, {glGetUniformLocation(id, ii.name.c_str()), ii.num_elements } });
		}
		auto frag_unis = frag_shader.GetUniforms();
		for (auto ii : frag_unis) {
			buffers.insert({ ii.name, {glGetUniformLocation(id, ii.name.c_str()), ii.num_elements } });
		}
	}

	void Use() const {
		glUseProgram(id);
	}

	GLuint GetId() const {
		return id;
	}

	void SetMatrixBuffer(std::string name, const GLfloat* data) {
		glUniformMatrix4fv(buffers.at(name).index, 1, GL_FALSE, data);
	}

	void SetVectorBuffer(const GLchar* name, const GLfloat* data) {

	}
};

