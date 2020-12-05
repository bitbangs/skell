#pragma once
#include <memory>
#include <vector>
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
	std::unique_ptr<Model<T>> model;
	GLuint texture_id;

public:
	Entity() = delete;
	Entity(std::shared_ptr<Mesh<T>> mesh,
		std::shared_ptr<Drawer<T>> drawer,
		//std::unique_ptr<Model<T>> model,
		T aspect_ratio, //should all models use the same aspect ratio?
		GLuint texture_id) :
		mesh(mesh),
		drawer(drawer),
		model(std::move(std::make_unique<Model<T>>(aspect_ratio))),
		texture_id(texture_id)
	{}
	
	Entity(std::shared_ptr<Mesh<T>> mesh,
		std::shared_ptr<Drawer<T>> drawer,
		std::vector<T> model_args, //can I do better than vector?
		GLuint texture_id) :
		mesh(mesh),
		drawer(drawer),
		model(std::make_unique<Model<T>>(model_args[0], model_args[1], model_args[2], model_args[3])),
		texture_id(texture_id)
	{}

	void Draw() {
		glBindVertexArray(mesh->GetVao()); //wasteful
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIbo()); //wasteful
		glBindTexture(GL_TEXTURE_2D, texture_id);
		drawer->GetShaderProgram().Use(); //wasteful
		drawer->GetShaderProgram().SetMatrixBuffer("mvp", model->GetMVP()); //wasteful
		glDrawElements(GL_TRIANGLES, mesh->GetNumIndices(), GL_UNSIGNED_INT, 0);
	}

	bool IsIntersecting(const Entity<T>& other) const {
		return model->IsIntersecting(*(other.model));
	}

	void Translate(LinearAlgebra::Vector<T> dt) {
		model->Translate(std::move(dt));
	}

	void TranslateTo(const Entity<T>& other) {
		model->TranslateTo(*(other.model));
	}
};
