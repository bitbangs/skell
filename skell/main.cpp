#include <algorithm>
#include <cmath>
#include <GL/glew.h>
#include <iostream>
#include <random>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <SDL.h>
#include <string>
#include <vector>
#include "Drawer.h"
#include "Mesh.hpp"
#include "ShaderProgram.h"
//shader factory pending :p
#include "VertexShader.h"
#include "FragmentShader.h"
#include "PPM.h"
#include "Obj.h"
#include "Entity.h"

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
	int width = 1680;
	int height = 1050;
	SDL_Window* window = SDL_CreateWindow(
		"sdl_window",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height,
		SDL_WINDOW_OPENGL
	);
	if (window == NULL) {
		logger->critical("could not create window");
		return 1;
	}
	GLfloat aspect_ratio = (GLfloat)width / (GLfloat)height;

	//gl context creation and glew initialization
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	glewInit();
	glEnable(GL_DEPTH_TEST);

	//vertex shader compilation
	VertexShader diffuse_vert_shader("#version 450\n"
		"in vec3 pos;\n"
		"in vec3 pass_norm;\n"
		"in vec2 pass_text;\n"
		"out vec4 norm;\n"
		"out vec4 frag_pos;\n"
		"out vec2 text;\n"
		"uniform mat4 mvp;\n"
		"void main() {\n"
		"gl_Position = mvp * vec4(pos, 1.0);\n"
		"text = pass_text;\n"
		"norm = vec4(pass_norm, 0.0);\n"
		"frag_pos = mvp * vec4(pos, 1.0);\n" //model may have non-uniform scaling (norm isn't perpendicular anymore)
	"}");
	//fragment shader compilation
	FragmentShader diffuse_frag_shader("#version 450\n"
		"in vec4 norm;\n"
		"in vec4 frag_pos;\n"
		"in vec2 text;\n"
		"out vec4 frag_color;\n"
		"uniform vec4 ambient;\n"
		"uniform vec4 light_pos;\n"
		"uniform sampler2D texture_image;\n"
		"void main() {\n"
		"vec4 light_dir = normalize(light_pos - frag_pos);\n"
		"vec4 norm_dir = normalize(norm);\n"
		"float diff = max(dot(norm_dir, light_dir), 0.0);\n"
		"vec4 diffuse = diff * vec4(1.1, 1.1, 1.1, 1.0);\n"
		"frag_color = ambient * diffuse * texture(texture_image, text);\n" 
	"}");

	//create an orange block texture
	GLuint orange_texture_id;
	glGenTextures(1, &orange_texture_id);
	glBindTexture(GL_TEXTURE_2D, orange_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	PPM orange_text("test.ppm");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, orange_text.GetWidth(), orange_text.GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, orange_text.GetData());
	glGenerateMipmap(GL_TEXTURE_2D);

	//create a blue block texture
	GLuint blue_texture_id;
	glGenTextures(1, &blue_texture_id);
	glBindTexture(GL_TEXTURE_2D, blue_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	PPM blue_text("skell_blue_test_texture.ppm");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, blue_text.GetWidth(), blue_text.GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, blue_text.GetData());
	glGenerateMipmap(GL_TEXTURE_2D);

	//load a sphere mesh
	Obj sphere_obj("sphere.obj");
	auto sphere = std::make_shared<Mesh<GLfloat>>(
		diffuse_vert_shader.GetAttributes(),
		sphere_obj.GetElements(),
		sphere_obj.GetIndices()
	);

	//create a block mesh
	Obj cube_obj("cube.obj");
	auto block = std::make_shared<Mesh<GLfloat>>(
		diffuse_vert_shader.GetAttributes(),
		cube_obj.GetElements(),
		cube_obj.GetIndices()
	);

	//create mesh drawer
	auto diffuse_drawer = std::make_shared<Drawer<GLfloat>>(ShaderProgram(diffuse_vert_shader, diffuse_frag_shader), aspect_ratio);

	//create the player
	Entity<GLfloat> player(sphere, 
		diffuse_drawer,
		{ aspect_ratio, +0.0f, -6.0f, +8.1f },
		{ 0.0f, 0.0f, 0.0f },
		10.0f,
		blue_texture_id
	);

	//brickbreaker bricks
	std::vector<Entity<GLfloat>> bricks;
	for (int ii = 0; ii < 16; ii += 2) {
		bricks.push_back({ block,
			diffuse_drawer,
			{ aspect_ratio, -8.0f + (GLfloat)ii, +1.0f, +8.1f },
			{ 0.0f, 0.0f, 0.0f },
			1.5f,
			blue_texture_id
		});
	}

	//wall bricks
	std::vector<Entity<GLfloat>> wall_bricks;
	for (int ii = 0; ii < 15; ++ii) { //back wall
		wall_bricks.push_back({ block,
			diffuse_drawer,
			{ aspect_ratio, -14.0f + (GLfloat)ii * 2.0f, +8.0f, +8.1f },
			{ 0.0f, 0.0f, 0.0f },
			2.0f,
			orange_texture_id
		});
	}
	for (int ii = 0; ii < 8; ++ii) { //left wall
		wall_bricks.push_back({ block,
			diffuse_drawer,
			{ aspect_ratio, -14.0f, -8.0f + (GLfloat)ii * 2.0f, +8.1f },
			{ 0.0f, 0.0f, 0.0f },
			2.0f,
			orange_texture_id
		});
	}
	for (int ii = 0; ii < 8; ++ii) { //right wall
		wall_bricks.push_back({ block,
			diffuse_drawer,
			{ aspect_ratio, +14.0f, +6.0f - (GLfloat)ii * 2.0f, +8.1f },
			{ 0.0f, 0.0f, 0.0f },
			2.0f,
			orange_texture_id
		});
	}

	//player can fire projectiles
	Entity<GLfloat> projectile(sphere,
		diffuse_drawer,
		aspect_ratio,
		{ 0.0f, 0.05f, 0.0f },
		0.5f,
		orange_texture_id
	);

	//translations (these should be controlled by the system...)
	GLfloat step = +0.15f;
	//GLfloat shoot = +0.05f; //this should be a vector so we can move in xx and yy
	//that will also allow for us to ricochet when intersecting
	//....but now we need to know which plane of the bounding box we hit
	//e.g.: projectile is moving diagonal up-right. intersection occurs.
	//was this due to hitting a bottom of a bounding box? keep xx and ricochet yy
	//was this due to hitting left side of a bounding box? ricochet xx and keep yy

	//^^^I'm starting to think we need another representation of each Model that deals with physics.
	//This new class can tell the model what transformations to make based on this new class'
	//state of velocity, forces, and probably much more.
	//The model shouldn't hold this stuff as it is really more of a bridge to opengl mvp of a "snapshot" for drawing
	//need to go look below to see where/how we tell Models to update

	//main loop events
	SDL_Event event;
	SDL_PollEvent(&event);
	unsigned char button_mask = 0;
	unsigned char dpad_mask = 0;
	bool quit = false;
	bool fire = false;

	//main loop
	while (!quit) {
		//route event
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
			case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
				button_mask |= 0x10;
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
			case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
				button_mask ^= 0x10;
				break;
			default:
				break;
			}
		}
		if (event.type == SDL_QUIT) {
			quit = true;
		}

		//process button events
		//instead of going directly to the models and translating them, we can create a physics representation
		//and impart a "button force" on that physics representation.  Let's admit, we'll probably call it Body.
		//so when we put forces on the Body, the body can later(??) sum them and give the model 1 transformation
		//so....does that also mean that Body's need to be able to talk to Models?
		//should we maybe make some sort of composite object?
		//Entity might be the place for this...currently we're dealing with, for example, player_model and player (Entity)
		//in main.cpp and these should probably be abstracted away into some single object to represent the player to main.cpp
		if (dpad_mask > 0) {
			switch (dpad_mask) {
			case 0x1: //2
				//player.Translate({ +0.0f, -step, +0.0f });
				player.ApplyForce({ +0.0f, -step, +0.0f }, 0.1f);
				break;
			case 0x2: //4
				//player.Translate({ -step, +0.0f, +0.0f });
				player.ApplyForce({ -step, +0.0f, +0.0f }, 0.1f);
				break;
			case 0x3: //1
				//player.Translate({ -step, -step, +0.0f });
				player.ApplyForce({ -step, -step, +0.0f }, 0.1f);
				break;
			case 0x4: //6
				//player.Translate({ +step, +0.0f, +0.0f });
				player.ApplyForce({ +step, +0.0f, +0.0f }, 0.1f);
				break;
			case 0x5: //3
				//player.Translate({ +step, -step, +0.0f });
				player.ApplyForce({ +step, -step, +0.0f }, 0.1f);
				break;
			case 0x8: //8
				//player.Translate({ +0.0f, +step, +0.0f });
				player.ApplyForce({ +0.0f, +step, +0.0f }, 0.1f);
				break;
			case 0xa: //7
				//player.Translate({ -step, +step, +0.0f });
				player.ApplyForce({ -step, +step, +0.0f }, 0.1f);
				break;
			case 0xc: //9
				//player.Translate({ +step, +step, +0.0f });
				player.ApplyForce({ +step, +step, +0.0f }, 0.1f);
				break;
			}
		}
		if (button_mask > 0) {
			switch (button_mask) {
			case 0x1:
				break;
			case 0x2:
				break;
			case 0x4:
				break;
			case 0x8:
				break;
			case 0x10:
				if (!fire) {
					fire = true;
					projectile.TranslateTo(player);
					projectile.Translate({ 0.0f, 1.4f, 0.0f });
				}
				break;
			}
		}

		//deal with projectile and collisions
		//this is what needs a heavy refactor
		player.Move();
		if (fire) {
			//check for collision with bricks
			auto dead_brick = std::remove_if(bricks.begin(), bricks.end(), [&](const auto& brick) {
				return projectile.IsIntersecting(brick);
				});
			if (dead_brick != bricks.end()) {
				bricks.erase(dead_brick);
				projectile.ScaleVelocity({ 0.0f, -1.0f, 0.0f });
			}

			//check for collision with wall
			for (const auto& wall_brick : wall_bricks) {
				if (projectile.IsIntersecting(wall_brick)) {
					projectile.ScaleVelocity({ 0.0f, -1.0f, 0.0f });
					break;
				}
			}

			//check for collision with paddle
			//if (projectile.IsIntersecting(player)) {
			//	projectile.ScaleVelocity({ 0.0f, -1.0f, 0.0f });
			//}
			player.Collide(projectile);

			projectile.Move();
		}

		//wipe frame
		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//draw the player
		player.Draw();
		//draw the bricks
		for (auto& brick : bricks) {
			brick.Draw();
		}
		//draw the wall
		for (auto& wall_brick : wall_bricks) {
			wall_brick.Draw();
		}
		//draw the projectile
		if (fire) {
			projectile.Draw();
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