#pragma once
#include <memory>
#include <vector>
#include "Drawer.h"
#include "FreeBody.hpp"
#include "Mesh.hpp"
#include "Model.hpp"
#include "ShaderProgram.h"

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
class Entity
{
private:
	std::shared_ptr<Mesh<T>> mesh; //shared because we'll reuse these
	std::shared_ptr<Drawer<T>> drawer; //shared because we'll reuse these
	std::unique_ptr<Model<T>> model; //unique because it is needed for drawing and absolute position
	std::shared_ptr<FreeBody<T>> free_body; //shared because we want to update these from an "all-knowing" collision class, but need to
	//know who actually owns which free body so it can be used to update models
	GLuint texture_id;

	bool IsIntersecting(const Entity<T>& other) const {
		//this really isn't good enough anymore because we need to know which face of the bounding box we
		//bump into. this will allow us to bounce off walls better
		return model->IsIntersecting(*(other.model));
	}

	void Translate(const LinearAlgebra::Vector<T>& dt) {
		model->Translate(dt);
	}

	void TranslateTo(const Entity<T>& other) {
		model->TranslateTo(*(other.model));
	}

public:
	Entity() = delete;
	Entity(std::shared_ptr<Mesh<T>> mesh,
		std::shared_ptr<Drawer<T>> drawer,
		T aspect_ratio, //should all models use the same aspect ratio?
		std::shared_ptr<FreeBody<T>> free_body,
		GLuint texture_id) :
		mesh(mesh),
		drawer(drawer),
		model(std::move(std::make_unique<Model<T>>(aspect_ratio))),
		free_body(free_body),
		texture_id(texture_id)
	{}
	Entity(std::shared_ptr<Mesh<T>> mesh,
		std::shared_ptr<Drawer<T>> drawer,
		std::vector<T> model_args, //can I do better than vector?
		std::shared_ptr<FreeBody<T>> free_body,
		GLuint texture_id) :
		mesh(mesh),
		drawer(drawer),
		model(std::make_unique<Model<T>>(model_args[0], model_args[1], model_args[2], model_args[3])),
		free_body(free_body),
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

//move/update needs to be here
	//model->Translate(free_body.velocity)

	void Fire(Entity<T>& projectile) {
		projectile.TranslateTo(*this);
		projectile.Translate({ 0.0f, 1.4f, 0.0f });
	}
};
