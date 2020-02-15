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
#include "Model.hpp"
#include "ShaderProgram.h"
//shader factory pending :p
#include "VertexShader.h"
#include "FragmentShader.h"

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
	VertexShader vert_shader("#version 450\n"
		"in vec3 pos;\n"
		"in vec4 pass_color;\n"
		"out vec4 color;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"void main() {\n"
		"gl_Position = projection * view * model * vec4(pos, 1.0);\n"
		"color = pass_color;\n"
		"}");

	//fragment shader compilation
	FragmentShader frag_shader("#version 450\n"
		"in vec4 color;\n"
		"out vec4 frag_color;\n"
		"uniform vec4 ambient;\n"
		"uniform vec4 light_pos;\n"
		"void main() {\n"
		"frag_color = ambient * color;"
		"}");

	//vertex shader compilation
	VertexShader red_vert_shader("#version 450\n"
		"in vec3 pos;\n"
		"in vec3 pass_norm;\n"
		"in vec4 pass_color;\n"
		"out vec4 color;\n"
		"out vec4 norm;\n"
		"out vec4 frag_pos;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"void main() {\n"
		"gl_Position = projection * view * model * vec4(pos, 1.0);\n"
		"color = pass_color;\n"
		"norm = vec4(pass_norm, 1.0);\n"
		"frag_pos = model * vec4(pos, 1.0);\n"
		"}");

	//fragment shader compilation
	FragmentShader red_frag_shader("#version 450\n"
		"in vec4 color;\n"
		"in vec4 norm;\n"
		"in vec4 frag_pos;\n"
		"out vec4 frag_color;\n"
		"uniform vec4 ambient;\n"
		"uniform vec4 light_pos;\n"
		"void main() {\n"
		"vec4 light_dir = normalize(light_pos - frag_pos);\n"
		"vec4 norm_dir = normalize(norm);\n"
		"float diff = max(dot(norm_dir, light_dir), 0.0);\n"
		"vec4 diffuse = diff * vec4(0.8, 0.8, 0.8, 1.0);\n"
		"frag_color = (ambient + diffuse) * color;"
		"}");

	//create an all red block
	Mesh<GLfloat> red_block(
		red_vert_shader.GetAttributes(),
		{
			//front face
			-0.5f, -0.5f, +0.0f, //0
			+0.0f, +0.0f, -1.0f, //toward camera
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			-0.5f, +0.5f, +0.0f, //1
			+0.0f, +0.0f, -1.0f, //toward camera
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, -0.5f, +0.0f, //2
			+0.0f, +0.0f, -1.0f, //toward camera
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			-0.5f, +0.5f, +0.0f, //1
			+0.0f, +0.0f, -1.0f, //toward camera
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, +0.5f, +0.0f, //3
			+0.0f, +0.0f, -1.0f, //toward camera
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, -0.5f, +0.0f, //2
			+0.0f, +0.0f, -1.0f, //toward camera
			+1.0f, +0.0f, +0.0f, +1.0f, //red

			//right face
			+0.5f, -0.5f, +0.0f, //2
			+1.0f, +0.0f, +0.0f, //toward right
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, +0.5f, +0.0f, //3
			+1.0f, +0.0f, +0.0f, //toward right
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, -0.5f, +1.0f, //4
			+1.0f, +0.0f, +0.0f, //toward right
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, +0.5f, +0.0f, //3
			+1.0f, +0.0f, +0.0f, //toward right
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, +0.5f, +1.0f, //5
			+1.0f, +0.0f, +0.0f, //toward right
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, -0.5f, +1.0f, //4
			+1.0f, +0.0f, +0.0f, //toward right
			+1.0f, +0.0f, +0.0f, +1.0f, //red

			//top face
			-0.5f, +0.5f, +0.0f, //0
			+0.0f, +1.0f, +0.0f, //up
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			-0.5f, +0.5f, +1.0f, //...
			+0.0f, +1.0f, +0.0f, //up
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, +0.5f, +0.0f, //...
			+0.0f, +1.0f, +0.0f, //up
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			-0.5f, +0.5f, +1.0f, //
			+0.0f, +1.0f, +0.0f, //up
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, +0.5f, +1.0f, //
			+0.0f, +1.0f, +0.0f, //up
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, +0.5f, +0.0f, //
			+0.0f, +1.0f, +0.0f, //up
			+1.0f, +0.0f, +0.0f, +1.0f, //red

			//left face
			-0.5f, -0.5f, +1.0f, //...
			-1.0f, +0.0f, +0.0f, //left
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, +0.5f, +1.0f, //...
			-1.0f, +0.0f, +0.0f, //left
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			-0.5f, -0.5f, +0.0f, //
			-1.0f, +0.0f, +0.0f, //left
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, +0.5f, +1.0f, //...
			-1.0f, +0.0f, +0.0f, //left
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			-0.5f, +0.5f, +0.0f, //
			-1.0f, +0.0f, +0.0f, //left
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			-0.5f, -0.5f, +0.0f, //
			-1.0f, +0.0f, +0.0f, //left
			+1.0f, +0.0f, +0.0f, +1.0f, //red

			//back face
			+0.5f, -0.5f, +1.0f, //
			+0.0f, +0.0f, +1.0f, //away
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, +0.5f, +1.0f, //
			+0.0f, +0.0f, +1.0f, //away
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			-0.5f, -0.5f, +1.0f, //
			+0.0f, +0.0f, +1.0f, //away
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, +0.5f, +1.0f, //
			+0.0f, +0.0f, +1.0f, //away
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			-0.5f, +0.5f, +1.0f, //
			+0.0f, +0.0f, +1.0f, //away
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			-0.5f, -0.5f, +1.0f, //
			+0.0f, +0.0f, +1.0f, //away
			+1.0f, +0.0f, +0.0f, +1.0f, //red

			//bottom face
			-0.5f, -0.5f, +1.0f, //
			+0.0f, -1.0f, +0.0f, //down
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			-0.5f, -0.5f, +0.0f, //
			+0.0f, -1.0f, +0.0f, //down
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, -0.5f, +1.0f, //
			+0.0f, -1.0f, +0.0f, //down
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			-0.5f, -0.5f, +0.0f, //
			+0.0f, -1.0f, +0.0f, //down
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, -0.5f, +0.0f, //
			+0.0f, -1.0f, +0.0f, //down
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			+0.5f, -0.5f, +1.0f, //
			+0.0f, -1.0f, +0.0f, //down
			+1.0f, +0.0f, +0.0f, +1.0f //red
		},
		{ //indices
			0u, 1u, 2u,
			3u, 4u, 5u,
			6u, 7u, 8u,
			9u, 10u, 11u,
			12u, 13u, 14u,
			15u, 16u, 17u,
			18u, 19u, 20u,
			21u, 22u, 23u,
			24u, 25u, 26u,
			27u, 28u, 29u,
			30u, 31u, 32u,
			33u, 34u, 35u
		}
		);

	//mesh data initialization
	Mesh<GLfloat> block(
		vert_shader.GetAttributes(),
		{
			-0.5f, -0.5f, +0.0f,
			+1.0f, +0.0f, +0.0f, +1.0f, //red

			-0.5f, +0.5f, +0.0f,
			+0.0f, +1.0f, +0.0f, +1.0f, //green

			+0.5f, -0.5f, 0.0f,
			+0.0f, +0.0f, +1.0f, +1.0f, //blue

			+0.5f, +0.5f, +0.0f,
			+1.0f, +0.0f, +1.0f, +1.0f, //purple
			//+1.0f, +0.0f, +1.0f,
			+0.5f, -0.5f, +1.0f,
			+1.0f, +0.0f, +0.0f, +1.0f, //red
			//+1.0f, +1.0f, +1.0f,
			+0.5f, +0.5f, +1.0f,
			+0.0f, +1.0f, +0.0f, +1.0f, //green
			//+0.0f, +0.0f, +1.0f,
			-0.5f, -0.5f, +1.0f,
			+0.0f, +0.0f, +1.0f, +1.0f, //blue
			//+0.0f, +1.0f, +1.0f,
			-0.5f, +0.5f, +1.0f,
			+1.0f, +0.0f, +1.0f, +1.0f //purple

		},
		{ //indices
			0u, 1u, 2u,
			1u, 3u, 2u,
			2u, 3u, 4u,
			3u, 5u, 4u,
			4u, 5u, 6u,
			5u, 7u, 6u,
			6u, 7u, 0u,
			7u, 1u, 0u,
			1u, 7u, 3u,
			7u, 5u, 3u,
			6u, 0u, 4u,
			0u, 2u, 4u
		}
		);

	//drawing classes
	Drawer red_block_drawer(ShaderProgram(red_vert_shader, red_frag_shader), aspect_ratio);
	Drawer block_drawer(ShaderProgram(vert_shader, frag_shader), aspect_ratio);

	//create the player
	Model<GLfloat> player;
	player.Translate(+0.0f, -6.0f, +8.1f);

	//spawn "enemies" with some uncertainty
	Model<GLfloat> spawned_model_i;
	Model<GLfloat> spawned_model_ii;
	Model<GLfloat> spawned_model_iii;
	Model<GLfloat> spawned_model_iv;
	std::random_device rand_dev;
	std::default_random_engine rand_eng(rand_dev());
	std::uniform_real_distribution<GLfloat> rand_uniform(+1.00f, +3.50f);

	//brickbreaker bricks
	std::vector<Model<GLfloat>> bricks;
	for (int ii = 0; ii < 8; ++ii) {
		Model<GLfloat> brick;
		brick.Translate(-6.0f + (GLfloat)(ii * 2), +1.0f, +8.1f);
		bricks.push_back(std::move(brick));
	}

	//player can fire projectiles
	Model<GLfloat> fire_model;

	//translations
	GLfloat step = +0.15f;
	GLfloat creep = +0.01f;
	GLfloat shoot = +0.05f;

	//main loop events
	SDL_Event event;
	SDL_PollEvent(&event);
	unsigned char button_mask = 0;
	unsigned char dpad_mask = 0;
	bool quit = false;
	unsigned char spawn_alive_mask = 0;
	bool fire = false;

	//main loop
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
			case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
				if (!fire) {
					fire = true;
					auto player_position = player.GetCentroid();
					fire_model = Model<GLfloat>();
					fire_model.Scale(+0.8f, +0.8f, +0.8f);
					fire_model.Translate(player_position[0], player_position[1], player_position[2]);
				}
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
				player.Translate(+0.0f, -step, +0.0f);
				break;
			case 0x2: //4
				player.Translate(-step, +0.0f, +0.0f);
				break;
			case 0x3: //1
				player.Translate(-step, -step, +0.0f);
				break;
			case 0x4: //6
				player.Translate(+step, +0.0f, +0.0f);
				break;
			case 0x5: //3
				player.Translate(+step, -step, +0.0f);
				break;
			case 0x8: //8
				player.Translate(+0.0f, +step, +0.0f);
				break;
			case 0xa: //7
				player.Translate(-step, +step, +0.0f);
				break;
			case 0xc: //9
				player.Translate(+step, +step, +0.0f);
				break;
			}
		}

		if (button_mask > 0) {
			switch (button_mask) {
			case 0x1: //x spawns in quadrant ii
				if ((spawn_alive_mask & 0x1) == 0) {
					spawned_model_ii = Model<GLfloat>();
					auto rand_xx = rand_uniform(rand_eng);
					auto rand_yy = rand_uniform(rand_eng);
					spawned_model_ii.Translate(-rand_xx, +rand_yy, +8.1f);
				}
				break;
			case 0x2: //y spawns in quadrant i
				if ((spawn_alive_mask & 0x2) == 0) {
					spawned_model_i = Model<GLfloat>();
					auto rand_xx = rand_uniform(rand_eng);
					auto rand_yy = rand_uniform(rand_eng);
					spawned_model_i.Translate(+rand_xx, +rand_yy, +8.1f);
				}
				break;
			case 0x4: //a spawns in quadrant iii
				if ((spawn_alive_mask & 0x4) == 0) {
					spawned_model_iii = Model<GLfloat>();
					auto rand_xx = rand_uniform(rand_eng);
					auto rand_yy = rand_uniform(rand_eng);
					spawned_model_iii.Translate(-rand_xx, -rand_yy, +8.1f);
				}
				break;
			case 0x8: //b spawns in quadrant iv
				if ((spawn_alive_mask & 0x8) == 0) {
					spawned_model_iv = Model<GLfloat>();
					auto rand_xx = rand_uniform(rand_eng);
					auto rand_yy = rand_uniform(rand_eng);
					spawned_model_iv.Translate(+rand_xx, -rand_yy, +8.1f);
				}
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
			spawn_alive_mask |= button_mask;
		}

		//wipe frame
		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//draw the player
		block_drawer.Draw(player.GetPointerToModelData(), block.GetNumIndices(), block.GetVao(), block.GetIbo());

		//draw the bricks
		for (const auto& brick : bricks) {
			red_block_drawer.Draw(brick.GetPointerToModelData(), red_block.GetNumIndices(), red_block.GetVao(), red_block.GetIbo());
		}

		//draw the enemies
		if (spawn_alive_mask > 0) {
			//move enemy toward player
			if (spawn_alive_mask & 0x1) {
				spawned_model_ii.MoveToward(player, creep);
				block_drawer.Draw(spawned_model_ii.GetPointerToModelData(), block.GetNumIndices(), block.GetVao(), block.GetIbo());
				//check if there was a collision
				if (fire && spawned_model_ii.IsIntersecting(fire_model)) {
					spawn_alive_mask ^= 0x1;
					fire = false;
				}
				else if (spawned_model_ii.IsIntersecting(player)) {
					quit = true;
				}
			}
			if (spawn_alive_mask & 0x2) {
				spawned_model_i.MoveToward(player, creep);
				block_drawer.Draw(spawned_model_i.GetPointerToModelData(), block.GetNumIndices(), block.GetVao(), block.GetIbo());
				//check if there was a collision
				if (fire && spawned_model_i.IsIntersecting(fire_model)) {
					spawn_alive_mask ^= 0x2;
					fire = false;
				}
				else if (spawned_model_i.IsIntersecting(player)) {
					quit = true;
				}
			}
			if (spawn_alive_mask & 0x4) {
				spawned_model_iii.MoveToward(player, creep);
				block_drawer.Draw(spawned_model_iii.GetPointerToModelData(), block.GetNumIndices(), block.GetVao(), block.GetIbo());
				//check if there was a collision
				if (fire && spawned_model_iii.IsIntersecting(fire_model)) {
					spawn_alive_mask ^= 0x4;
					fire = false;
				}
				else if (spawned_model_iii.IsIntersecting(player)) {
					quit = true;
				}
			}
			if (spawn_alive_mask & 0x8) {
				spawned_model_iv.MoveToward(player, creep);
				block_drawer.Draw(spawned_model_iv.GetPointerToModelData(), block.GetNumIndices(), block.GetVao(), block.GetIbo());
				//check if there was a collision
				if (fire && spawned_model_iv.IsIntersecting(fire_model)) {
					spawn_alive_mask ^= 0x8;
					fire = false;
				}
				else if (spawned_model_iv.IsIntersecting(player)) {
					quit = true;
				}
			}
		}

		//draw the projectile
		if (fire) {
			fire_model.Translate(+0.0f, +shoot, +0.0f);
			block_drawer.Draw(fire_model.GetPointerToModelData(), block.GetNumIndices(), block.GetVao(), block.GetIbo());
			if (fire_model.GetCentroid()[1] > +5.0f) {
				fire = false;
			}
			//check for collision with bricks
			auto dead_brick = std::remove_if(bricks.begin(), bricks.end(), [&](const auto& brick){
				return fire_model.IsIntersecting(brick);
			});
			if (dead_brick != bricks.end()) {
				bricks.erase(dead_brick);
				fire = false;
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