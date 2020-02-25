#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <GL/glew.h>

class Obj
{
private:
	std::vector<GLfloat> elements;
	std::vector<GLuint> indices;

public:
	Obj() = delete;
	Obj(const char* file_name) {
		std::ifstream obj_file(file_name);
		if (obj_file.is_open()) {
			std::vector<GLfloat> vertices;
			std::vector<GLfloat> textures;
			std::vector<GLfloat> normals;
			std::string line = "";
			while (line != "v") { //skip to vertices
				obj_file >> line;
			}
			GLuint index = 0;
			while (line == "v") { //load all vertices
				obj_file >> line; //xx
				vertices.push_back(std::stof(line));
				obj_file >> line; //yy
				vertices.push_back(std::stof(line));
				obj_file >> line; //zz
				vertices.push_back(std::stof(line));
				obj_file >> line; //next line
			}
			while (line == "vt") { //load all texture coords
				obj_file >> line; //ss
				textures.push_back(std::stof(line));
				obj_file >> line; //tt
				textures.push_back(std::stof(line));
				obj_file >> line; //next line
			}
			while (line == "vn") {
				obj_file >> line; //xx
				normals.push_back(std::stof(line));
				obj_file >> line; //yy
				normals.push_back(std::stof(line));
				obj_file >> line; //zz
				normals.push_back(std::stof(line));
				obj_file >> line; //next line
			}
			while (line != "f") { //skip to faces
				obj_file >> line;
			}
			while (line == "f") {
				obj_file >> line; //vert0
				indices.push_back(index++);
				size_t first_slash = line.find_first_of('/');
				size_t second_slash = line.find_last_of('/');
				int vert_offset = std::stoi(line.substr(0, first_slash)) - 1;
				elements.push_back(vertices[3 * vert_offset]);
				elements.push_back(vertices[3 * vert_offset + 1]);
				elements.push_back(vertices[3 * vert_offset + 2]);
				int norm_offset = std::stoi(line.substr(second_slash + 1)) - 1;
				elements.push_back(normals[3 * norm_offset]);
				elements.push_back(normals[3 * norm_offset + 1]);
				elements.push_back(normals[3 * norm_offset + 2]);
				int text_offset = std::stoi(line.substr(first_slash + 1, second_slash - first_slash - 1)) - 1;
				elements.push_back(textures[2 * text_offset]);
				elements.push_back(textures[2 * text_offset + 1]);

				obj_file >> line; //vert1
				indices.push_back(index++);
				first_slash = line.find_first_of('/');
				second_slash = line.find_last_of('/');
				vert_offset = std::stoi(line.substr(0, first_slash)) - 1;
				elements.push_back(vertices[3 * vert_offset]);
				elements.push_back(vertices[3 * vert_offset + 1]);
				elements.push_back(vertices[3 * vert_offset + 2]);
				norm_offset = std::stoi(line.substr(second_slash + 1)) - 1;
				elements.push_back(normals[3 * norm_offset]);
				elements.push_back(normals[3 * norm_offset + 1]);
				elements.push_back(normals[3 * norm_offset + 2]);
				text_offset = std::stoi(line.substr(first_slash + 1, second_slash - first_slash - 1)) - 1;
				elements.push_back(textures[2 * text_offset]);
				elements.push_back(textures[2 * text_offset + 1]);

				obj_file >> line; //vert2
				indices.push_back(index++);
				first_slash = line.find_first_of('/');
				second_slash = line.find_last_of('/');
				vert_offset = std::stoi(line.substr(0, first_slash)) - 1;
				elements.push_back(vertices[3 * vert_offset]);
				elements.push_back(vertices[3 * vert_offset + 1]);
				elements.push_back(vertices[3 * vert_offset + 2]);
				norm_offset = std::stoi(line.substr(second_slash + 1)) - 1;
				elements.push_back(normals[3 * norm_offset]);
				elements.push_back(normals[3 * norm_offset + 1]);
				elements.push_back(normals[3 * norm_offset + 2]);
				text_offset = std::stoi(line.substr(first_slash + 1, second_slash - first_slash - 1)) - 1;
				elements.push_back(textures[2 * text_offset]);
				elements.push_back(textures[2 * text_offset + 1]);
				obj_file >> line; //next line
				
			}
			obj_file.close();
		}
	}

	std::vector<GLfloat> GetElements() const {
		return elements;
	}
	std::vector<GLuint> GetIndices() const {
		return indices;
	}
};

