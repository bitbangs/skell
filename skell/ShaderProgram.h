#pragma once
#include <initializer_list>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <vector>
#include "VertexShader.h"
#include "FragmentShader.h"

class ShaderProgram
{
private:
	GLuint id;
	std::shared_ptr<spdlog::logger> logger;

public:
	ShaderProgram() = delete;
	ShaderProgram(VertexShader vert_shader, FragmentShader frag_shader) :
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
		auto attribs = vert_shader.GetAttributes();
		for (int ii = 0; ii < attribs.size(); ++ii) {
			glBindAttribLocation(id, ii, attribs[ii].name);
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
	}

	void Use() const {
		glUseProgram(id);
	}

	//this is temporary
	GLuint GetId() const {
		return id;
	}
};

