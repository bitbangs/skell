#include <GL/glew.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <SDL.h>
#include <string>
#include <vector>

#include <LinearAlgebra/Vector.hpp>
#include <LinearAlgebra/Matrix.hpp>
#include "Model.hpp"

int main(int argc, char* argv[]) {
	//logger initialization
	std::shared_ptr<spdlog::logger> logger;
	try {
		logger = spdlog::basic_logger_mt("file_logger", "log.txt");
	}
	catch (const spdlog::spdlog_ex & ex) {
		std::cout << "spdlog init failed: " << ex.what() << '\n';
	}

	//sdl initialization
	uint32_t sdl_init_flags = SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER;
	if (SDL_Init(sdl_init_flags) != 0) {
		logger->critical("could not initialize sdl");
		return 1;
	}

	//controller inspection
	int num_joysticks = SDL_NumJoysticks();
	if (num_joysticks < 0) {
		logger->warn("no joysticks found");
	}
	std::vector<SDL_GameController*> joysticks(num_joysticks); //raw pointer...can this change?
	for (int ii = 0; ii < num_joysticks; ++ii) {
		SDL_GameController* joystick = SDL_GameControllerOpen(ii); //raw pointer...can this change?
		const char* joystick_name = SDL_GameControllerName(joystick);
		if (joystick_name == NULL) {
			logger->warn("could not get controller name");
		}
		else {
			logger->info(joystick_name);
			joysticks.push_back(joystick);
		}
	}

	//sdl window creation
	SDL_Window* window = SDL_CreateWindow(
		"sdl_window",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		640, 640,
		SDL_WINDOW_OPENGL
	);
	if (window == NULL) {
		logger->critical("could not create window");
		return 1;
	}

	//gl context creation and glew initialization
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	glewInit();

	//vertex shader compilation
	GLuint vert_shader_id = glCreateShader(GL_VERTEX_SHADER);
	const GLchar* vert_shader_src = "#version 450\n"
		"in vec3 pos;\n"
		"in vec4 pass_color;\n"
		"out vec4 color;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"void main() {\n"
		"gl_Position = view * model * vec4(pos, 1.0);\n"
		"color = pass_color;\n"
	"}";
	glShaderSource(vert_shader_id, 1, &vert_shader_src, NULL);
	glCompileShader(vert_shader_id);
	GLint is_vert_shader_compiled = GL_FALSE;
	glGetShaderiv(vert_shader_id, GL_COMPILE_STATUS, &is_vert_shader_compiled);
	if (is_vert_shader_compiled == GL_FALSE) {
		GLint err_msg_size = 0;
		glGetShaderiv(vert_shader_id, GL_INFO_LOG_LENGTH, &err_msg_size);
		GLchar* err_msg = new GLchar[err_msg_size];
		glGetShaderInfoLog(vert_shader_id, err_msg_size, NULL, err_msg);

		logger->warn("vertex shader did not compile");
		if (err_msg == NULL) {
			logger->warn("could not get vertex shader compilation error message");
		}
		else {
			logger->warn(err_msg);
		}
		delete[] err_msg;
	}
	logger->info("vertex shader compiled successfully");
	//fragment shader compilation
	GLuint frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar* frag_shader_src = "#version 450\n"
		"in vec4 color;\n"
		"out vec4 frag_color;\n"
		"uniform vec4 ambient;\n"
		"void main() {\n"
		"frag_color = ambient * color;"
		"}";
	glShaderSource(frag_shader_id, 1, &frag_shader_src, NULL);
	glCompileShader(frag_shader_id);
	GLint is_frag_shader_compiled = GL_FALSE;
	glGetShaderiv(frag_shader_id, GL_COMPILE_STATUS, &is_frag_shader_compiled);
	if (is_frag_shader_compiled == GL_FALSE) {
		GLint err_msg_size = 0;
		glGetShaderiv(frag_shader_id, GL_INFO_LOG_LENGTH, &err_msg_size);
		GLchar* err_msg = new GLchar[err_msg_size];
		glGetShaderInfoLog(frag_shader_id, err_msg_size, NULL, err_msg);

		logger->warn("fragment shader did not compile");
		if (err_msg == NULL) {
			logger->warn("could not get fragment shader compilation error message");
		}
		else {
			logger->warn(err_msg);
		}
		delete[] err_msg;
	}
	logger->info("fragment shader compiled successfully");

	//vertex data initialization for triangle
	GLuint vbo;
	glGenBuffers(1, &vbo); //get a vbo from opengl
	GLfloat mesh[] = {
		+0.0f, +0.0f, +0.0f,
		+1.0f, +0.0f, +0.0f, +1.0f, //red
		+0.0f, +1.0f, +0.0f,
		+0.0f, +1.0f, +0.0f, +1.0f, //green
		+1.0f, +0.0f, +0.0f,
		+0.0f, +0.0f, +1.0f, +1.0f, //blue
		+1.0f, +1.0f, +0.0f,
		+1.0f, +0.0f, +1.0f, +1.0f //purple
	};
	glBindBuffer(GL_ARRAY_BUFFER, vbo); //must bind so next call knows where to put data
	glBufferData(GL_ARRAY_BUFFER, //glBufferData is used for mutable storage
		sizeof(mesh), //sizeof(positions), //size in bytes
		&mesh, //&positions, //const void*
		GL_STATIC_DRAW
	);
	GLuint vao;
	glGenVertexArrays(1, &vao); //get a vao from opengl
	glBindVertexArray(vao); //must bind vao before configuring it
	glVertexAttribPointer(0, //attribute index 0
		3, GL_FLOAT, //vbo is already bound in current state; contains 3 floats for each vertex position
		GL_FALSE, //do not normalize
		7 * sizeof(GLfloat), //stride in bytes
		0 //no offset
	);
	glEnableVertexAttribArray(0); //must enable attribute 0 (positions)
	glVertexAttribPointer(1, //attribute index 1
		4, GL_FLOAT, //vbo is already bound in current state; contains 4 floats for each vertex color, rgba
		GL_FALSE, //do not normalize
		7 * sizeof(GLfloat), //stride in bytes
		(void*)(3 * sizeof(GLfloat)) //byte offset
	);
	glEnableVertexAttribArray(1); //must enable attribute 0 (positions)

	//create indices, which must be done after vao is bound
	GLuint ibo;
	glGenBuffers(1, &ibo);
	GLuint indices[] = {
		0u, 1u, 2u,
		1u, 3u, 2u
	};
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); //binding here attaches us to vao
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(indices),
		&indices,
		GL_STATIC_DRAW
	);

	//bind to a program before you link
	GLuint program = glCreateProgram();
	glAttachShader(program, vert_shader_id);
	glAttachShader(program, frag_shader_id);
	glBindAttribLocation(program, 0, "pos"); //bind attribute 0 to "pos" shader variable
	glBindAttribLocation(program, 1, "pass_color"); //bind attribute 1 to "color"
	glLinkProgram(program);
	GLint is_program_linked = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &is_program_linked);
	if (is_program_linked == GL_FALSE) {
		GLint err_msg_size = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &err_msg_size);

		GLchar* err_msg = new GLchar[err_msg_size];
		glGetProgramInfoLog(program, err_msg_size, NULL, err_msg);

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
	glUseProgram(program);

	//create multiple instances of the mesh via the model uniform
	GLint model_id = glGetUniformLocation(program, "model");
	/*Model<GLfloat> model({
		+0.25f, +0.00f, +0.00f, +0.00f,
		+0.00f, +0.25f, +0.00f, +0.00f,
		+0.00f, +0.00f, +0.25f, +0.00f,
		+0.00f, +0.00f, +0.00f, +1.00f
	});*/
	Model<GLfloat> model;
	//model.Scale(+0.25f, +0.25f, +0.25f);
	glUniformMatrix4fv(model_id, 1, GL_FALSE, model.GetPointerToData());

	//view matrix for the eye
	GLint view_id = glGetUniformLocation(program, "view");
	//Model<GLfloat> view({
	LinearAlgebra::Matrix<GLfloat> left_view(4, 4, {
		+1.0f, +0.0f, +0.0f, +0.0f, //our right
		+0.0f, +1.0f, +0.0f, +0.0f, //up
		+0.0f, +0.0f, +1.0f, +0.0f, //look in this direction
		+0.0f, +0.0f, +0.0f, +1.0f
	});
	LinearAlgebra::Matrix<GLfloat> right_view(4, 4, {
		+1.0f, +0.0f, +0.0f, +0.0f,
		+0.0f, +1.0f, +0.0f, +0.0f,
		+0.0f, +0.0f, +1.0f, +0.0f,
		+0.0f, +0.0f, +0.5f, +1.0f //our position
	});
	LinearAlgebra::Matrix<GLfloat> view = right_view * left_view;
	glUniformMatrix4fv(view_id, 1, GL_FALSE, view.GetPointerToData());

	//projection matrix
	

	//set the ambient light
	GLint ambient_id = glGetUniformLocation(program, "ambient");
	GLfloat ambient = 0.6f;
	glUniform4f(ambient_id, ambient, ambient, ambient, 1.0f);

	//main loop
	SDL_Event event;
	SDL_PollEvent(&event);
	unsigned char button_mask = 0;
	unsigned char dpad_mask = 0;
	bool quit = false;
	unsigned char spawn_alive_mask = 0;
	Model<GLfloat> spawned_model;
	//spawned_model.Scale(+0.15f, +0.15f, +0.15f);
	while (!quit) {
		if (event.type == SDL_CONTROLLERBUTTONDOWN) {
			auto button = event.cbutton.button;
			switch (button) {
			case SDL_CONTROLLER_BUTTON_X:
				button_mask |= 0x1;
				break;
			case SDL_CONTROLLER_BUTTON_Y:
				button_mask |= 0x2;
				break;
			case SDL_CONTROLLER_BUTTON_A:
				button_mask |= 0x4;
				break;
			case SDL_CONTROLLER_BUTTON_B:
				button_mask |= 0x8;
				break;
			case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
				dpad_mask |= 0x2;
				break;
			case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
				dpad_mask |= 0x4;
				break;
			case SDL_CONTROLLER_BUTTON_DPAD_UP:
				dpad_mask |= 0x8;
				break;
			case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
				dpad_mask |= 0x1;
				break;
			case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
				quit = true;
				break;
			default:
				break;
			}
		}
		if (event.type == SDL_CONTROLLERBUTTONUP) {
			auto button = event.cbutton.button;
			switch (button) {
			case SDL_CONTROLLER_BUTTON_X:
				button_mask ^= 0x1;
				break;
			case SDL_CONTROLLER_BUTTON_Y:
				button_mask ^= 0x2;
				break;
			case SDL_CONTROLLER_BUTTON_A:
				button_mask ^= 0x4;
				break;
			case SDL_CONTROLLER_BUTTON_B:
				button_mask ^= 0x8;
				break;
			case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
				dpad_mask ^= 0x2;
				break;
			case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
				dpad_mask ^= 0x4;
				break;
			case SDL_CONTROLLER_BUTTON_DPAD_UP:
				dpad_mask ^= 0x8;
				break;
			case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
				dpad_mask ^= 0x1;
				break;
			default:
				break;
			}
		}
		if (event.type == SDL_QUIT) {
			quit = true;
		}

		if (dpad_mask > 0) {
			switch (dpad_mask) {
			case 0x1: //2
				model.Translate(+0.0f, -0.1f, +0.0f);
				break;
			case 0x2: //4
				model.Translate(-0.1f, +0.0f, +0.0f);
				break;
			case 0x3: //1
				model.Translate(-0.1f, -0.1f, +0.0f);
				break;
			case 0x4: //6
				model.Translate(+0.1f, +0.0f, +0.0f);
				break;
			case 0x5: //3
				model.Translate(+0.1f, -0.1f, +0.0f);
				break;
			case 0x8: //8
				model.Translate(+0.0f, +0.1f, +0.0f);
				break;
			case 0xa: //7
				model.Translate(-0.1f, +0.1f, +0.0f);
				break;
			case 0xc: //9
				model.Translate(+0.1f, +0.1f, +0.0f);
				break;
			}
			glUniformMatrix4fv(model_id, 1, GL_FALSE, model.GetPointerToData());
		}

		if (button_mask > 0) { //maybe later find a way to separate input from drawing better
			switch (button_mask) {
			case 0x1: //x
				spawned_model.Translate(-0.15f, +0.00f, +0.00f);
				break;
			case 0x2: //y
				spawned_model.Translate(+0.00f, +0.15f, +0.00f);
				break;
			case 0x4: //a
				spawned_model.Translate(+0.00f, -0.15f, +0.00f);
				break;
			case 0x8: //b
				spawned_model.Translate(+0.15f, +0.00f, +0.00f);
				break;
				//case 0x3: //x and y
				//	break;
				//case 0x5: //x and a
				//	break;
				//case 0x6: //y and a
				//	break;
				//case 0x9: //x and b
				//	break;
				//case 0xa: //y and b
				//	break;
				//case 0xc: //a and b
				//	break;
				//case 0x7: //x, y, and a
				//	break;
				//case 0xb: //x, y, and b
				//	break;
				//case 0xd: //x, a, and b
				//	break;
				//case 0xe: //y, a, and b
				//	break;
				//case 0xf: //x, a, b, and y
				//	break;
			}
			spawn_alive_mask = button_mask;
		}

		//wipe frame
		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//drawing begins
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); //draw the main square mesh
		
		if (spawn_alive_mask > 0) {
			glUniformMatrix4fv(model_id, 1, GL_FALSE, spawned_model.GetPointerToData());
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); //draw a spawned enemy (or whatever)
			glUniformMatrix4fv(model_id, 1, GL_FALSE, model.GetPointerToData()); //set player model back

			//check if there was a collision
			auto spawned_xx = spawned_model.Getxx();
			auto spawned_yy = spawned_model.Getyy();
			auto spawned_sx = spawned_model.Getsx();
			auto spawned_sy = spawned_model.Getsy();
			auto xx = model.Getxx();
			auto yy = model.Getyy();
			auto sx = model.Getsx();
			auto sy = model.Getsy();

			//this is not correct, but kinda works
			if ((spawned_xx + spawned_sx >= xx && spawned_xx <= xx + sx)) {
				if (spawned_yy + spawned_sy >= yy && spawned_yy <= yy + sy) {
					spawn_alive_mask = 0;
				}
			}
		}

		//progress
		SDL_GL_SwapWindow(window);
		SDL_PollEvent(&event);
	}

	//clean up
	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}