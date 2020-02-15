#pragma once
#include <algorithm>
#include <type_traits>
#include <vector>
#include <GL/glew.h>
#include "Attribute.h"

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
class Mesh {
private:
	std::vector<Attribute> attribs;
	std::unique_ptr<T[]> elements;
	std::unique_ptr<GLuint[]> indices;
	GLsizei stride;
	GLsizei num_indices;
	GLuint vao;
	GLuint ibo;

public:
	Mesh() = delete;
	Mesh(std::vector<Attribute> attribs,
		std::initializer_list<T> element_list,
		std::initializer_list<GLuint> index_list) :
		attribs(attribs),
		elements(std::make_unique<T[]>(element_list.size())),
		indices(std::make_unique<GLuint[]>(index_list.size())),
		stride(0),
		num_indices(index_list.size())
	{
		for (size_t ii = 0; ii < attribs.size(); ++ii) {
			stride += attribs[ii].num_elements;
		}
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
			sizeof(elements) * element_list.size(), //size in bytes
			elements.get(), //const void*
			GL_STATIC_DRAW //will these always be static_draw?
		);
		glGenVertexArrays(1, &vao); //get a vao from opengl
		glBindVertexArray(vao); //must bind vao before configuring it
		int offset = 0;
		for (GLuint ii = 0; ii < attribs.size(); ++ii) {
			//glEnableVertexAttribArray(ii); //must enable attribute ii
			glVertexAttribPointer(ii, //attribute index ii
				attribs[ii].num_elements, //vbo is already bound in current state; contains 3 floats for each vertex position
				GL_FLOAT, //get this from T?
				GL_FALSE, //do not normalize
				stride * sizeof(T), //stride in bytes
				(void*)(offset * sizeof(T)) //byte offset
			);
			glEnableVertexAttribArray(ii); //must enable attribute ii
			offset += attribs[ii].num_elements;
		}
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); //binding here attaches us to vao
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(indices) * index_list.size(),
			indices.get(),
			GL_STATIC_DRAW
		);
	}

	GLsizei GetNumIndices() const {
		return num_indices;
	}

	GLuint GetVao() const {
		return vao;
	}

	GLuint GetIbo() const {
		return ibo;
	}
};