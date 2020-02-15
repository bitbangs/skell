#pragma once
#include <GL/glew.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <string>
#include <sstream>
#include <vector>

struct Uniform {
	//const GLchar* name;
	std::string name;
	const GLsizei num_elements;
};

class Shader
{
private:
	std::shared_ptr<spdlog::logger> logger;
	std::vector<Uniform> uniforms;

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

		//shader compilation
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

		//parse out uniforms
		std::stringstream shader_file(src);
		std::string line;
		shader_file >> line;
		GLuint index = 0;
		while (line != "void") { //until start of main() signature
			if (line == "uniform") {
				shader_file >> line; //get type
				GLsizei num_elements = (GLsizei)(line.back() - 48);
				if (line.front() == 'm') {
					num_elements *= num_elements; //square matrix
				}
				shader_file >> line; //get name
				line.pop_back(); //remove ';'
				//const GLchar* name = line.c_str();
				//const GLint uniform_id = glGetUniformLocation(id, name);
				uniforms.push_back({line, num_elements});
			}
			shader_file >> line;
		}
	}

public:
	Shader() = delete;

	const GLuint GetId() const {
		return id;
	}

	std::vector<Uniform> GetUniforms() const {
		return uniforms;
	}
};

