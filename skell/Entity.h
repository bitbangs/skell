#pragma once
#include <memory>
#include "Drawer.h"
#include "Mesh.hpp"
#include "Model.hpp"
#include "ShaderProgram.h"

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
class Entity
{
private:
	std::shared_ptr<Mesh<T>> mesh;
	std::shared_ptr<Drawer<T>> drawer;
	std::shared_ptr<Model<T>> model;
	GLuint texture_id;

public:
	Entity() = delete;
	Entity(std::shared_ptr<Mesh<T>> mesh,
		std::shared_ptr<Drawer<T>> drawer,
		std::shared_ptr<Model<T>> model,
		GLuint texture_id) :
		mesh(mesh),
		drawer(drawer),
		model(model),
		texture_id(texture_id)
	{}

	void Draw() {
		glBindVertexArray(mesh->GetVao()); //wasteful
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIbo()); //wasteful
		glBindTexture(GL_TEXTURE_2D, texture_id);
		drawer->GetShaderProgram().Use(); //wasteful
		drawer->GetShaderProgram().SetMatrixBuffer("mvp", model->GetMVP()); //wasteful
		//drawer->GetShaderProgram().SetMatrixBuffer("model", model->GetModel());
		glDrawElements(GL_TRIANGLES, mesh->GetNumIndices(), GL_UNSIGNED_INT, 0);
	}

	bool IsIntersecting(const Entity<T>& other) const {
		return model->IsIntersecting(*(other.model));
	}
};

