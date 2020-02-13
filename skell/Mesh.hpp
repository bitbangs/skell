#pragma once
#include <algorithm>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <GL/glew.h>

struct Attribute {
	const std::string name;
	const GLuint index;
	const GLint num_elements;
};

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
class Mesh {
private:
	//std::unique_ptr<Attribute[]> attribs;
	std::unique_ptr<T[]> elements;
	std::unique_ptr<GLuint[]> indices;
	GLsizei stride;

public:
	Mesh() = delete;
	Mesh(std::initializer_list<Attribute> attrib_list,
		std::initializer_list<T> element_list,
		std::initializer_list<GLuint> index_list,
		const GLsizei stride_elements) :
		//attribs(std::make_unique<Attribute[]>(attrib_list.size())),
		elements(std::make_unique<T[]>(element_list.size())),
		indices(std::make_unique<GLuint[]>(index_list.size())),
		stride(stride_elements)
	{
		for (size_t ii = 0; ii < element_list.size(); ++ii) {
			elements[ii] = *(element_list.begin() + ii);
		}
		for (size_t ii = 0; ii < index_list.size(); ++ii) {
			indices[ii] = *(index_list.begin() + ii);
		}

		//give to opengl
		GLuint vbo;
		glGenBuffers(1, &vbo); //get a vbo from opengl
		glBindBuffer(GL_ARRAY_BUFFER, vbo); //must bind so next call knows where to put data
		glBufferData(GL_ARRAY_BUFFER, //glBufferData is used for mutable storage
			sizeof(elements) * element_list.size(), //sizeof(positions), //size in bytes
			elements.get(), //&positions, //const void*
			GL_STATIC_DRAW
		);
		GLuint vao;
		glGenVertexArrays(1, &vao); //get a vao from opengl
		glBindVertexArray(vao); //must bind vao before configuring it
		int offset = 0;
		for (GLuint ii = 0; ii < attrib_list.size(); ++ii) {
			glVertexAttribPointer(ii, //attribute index ii
				(attrib_list.begin() + ii)->num_elements, //vbo is already bound in current state; contains 3 floats for each vertex position
				GL_FLOAT, //get this from T?
				GL_FALSE, //do not normalize
				stride * sizeof(T), //stride in bytes
				(void*)(offset * sizeof(T)) //byte offset
			);
			glEnableVertexAttribArray(ii); //must enable attribute ii
			offset += (attrib_list.begin() + ii)->num_elements;
		}
		GLuint ibo;
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); //binding here attaches us to vao
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(indices) * index_list.size(),
			indices.get(),
			GL_STATIC_DRAW
		);
	}

	//need to expose attributes...
};