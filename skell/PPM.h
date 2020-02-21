#pragma once
#include <GL/glew.h>
#include <fstream>
#include <memory>
#include <string>

class PPM
{
private:
	std::unique_ptr<char[]> data;
	GLint width;
	GLint height;

public:
	PPM() = delete;
	PPM(const char* file_name) {
		std::ifstream image(file_name);
		if (image.is_open()) {
			std::string line;
			std::getline(image, line); //skip version
			std::getline(image, line);
			while (line.front() == '#') { //skip comments
				std::getline(image, line);
			}
			//get width and height
			std::size_t space_index = line.find_first_of(" ", 0);
			width = std::stoi(line.substr(0, space_index));
			height = std::stoi(line.substr(space_index, line.size() - space_index));
			std::getline(image, line); //skip bit depth, just assume 3

			data = std::make_unique<char[]>(width * height * 3);
			image.read(data.get(), width * height * 3);

			image.close();
		}
	}

	GLuint GetWidth() const {
		return width;
	}
	GLuint GetHeight() const {
		return height;
	}

	const char* GetData() const {
		return data.get();
	}

	void WriteOutTest(const char* file_name) {
		std::ofstream image(file_name);
		if (image.is_open()) {
			image << "P6\n";
			image << "# are we good?\n";
			image << width << ' ' << height << '\n';
			image << "255\n";
			image.write(data.get(), width * height * 3);
			image.close();
		}
	}
};

