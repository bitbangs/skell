#pragma once
#include <GL/glew.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

class Shader
{
private:
	std::shared_ptr<spdlog::logger> logger;

protected:
	GLuint id;
	Shader(const GLchar* src, GLuint id) :
		id(id)
	{
		//logger initialization
		std::string name = "shader" + std::to_string(id);
		try {
			logger = spdlog::basic_logger_mt(name + "_logger", name + "_log.txt");
		}
		catch (const spdlog::spdlog_ex & ex) {
			std::cout << "spdlog init failed: " << ex.what() << '\n';
		}

		glShaderSource(id, 1, &src, NULL);
		glCompileShader(id);
		GLint is_shader_compiled = GL_FALSE;
		glGetShaderiv(id, GL_COMPILE_STATUS, &is_shader_compiled);
		if (is_shader_compiled == GL_FALSE) {
			GLint err_msg_size = 0;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &err_msg_size);
			GLchar* err_msg = new GLchar[err_msg_size];
			glGetShaderInfoLog(id, err_msg_size, NULL, err_msg);

			logger->warn(name + "shader did not compile");
			if (err_msg == NULL) {
				logger->warn("could not get " + name + " shader compilation error message");
			}
			else {
				logger->warn(err_msg);
			}
			delete[] err_msg;
		}
		logger->info(name + " compiled successfully");
	}

public:
	Shader() = delete;

	const GLuint GetId() const {
		return id;
	}
};

