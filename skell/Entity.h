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
	LinearAlgebra::Vector<T> velocity; //lets soon replace this with a mass
	T mass; //or add the mass in...can I really factor velocity out? I don't think so...
	GLuint texture_id;

public:
	Entity() = delete;
	Entity(std::shared_ptr<Mesh<T>> mesh,
		std::shared_ptr<Drawer<T>> drawer,
		T aspect_ratio, //should all models use the same aspect ratio?
		std::initializer_list<T> velocity,
		T mass,
		GLuint texture_id) :
		mesh(mesh),
		drawer(drawer),
		model(std::move(std::make_unique<Model<T>>(aspect_ratio))),
		velocity(velocity),
		mass(mass),
		texture_id(texture_id)
	{}
	Entity(std::shared_ptr<Mesh<T>> mesh,
		std::shared_ptr<Drawer<T>> drawer,
		std::vector<T> model_args, //can I do better than vector?
		std::initializer_list<T> velocity,
		T mass,
		GLuint texture_id) :
		mesh(mesh),
		drawer(drawer),
		model(std::make_unique<Model<T>>(model_args[0], model_args[1], model_args[2], model_args[3])),
		velocity(velocity),
		mass(mass),
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

	void Collide(Entity<T>& other) {
		if (this->IsIntersecting(other)) {
			other.ApplyForce(velocity.Scale(1.0f), 0.1f); //scale by 1.0f is just a hack right now to get around not having a copy constructor for Vectorh
		}
	}

	void Translate(const LinearAlgebra::Vector<T>& dt) {
		model->Translate(dt);
	}

	void TranslateTo(const Entity<T>& other) {
		model->TranslateTo(*(other.model));
	}

	void ApplyForce(LinearAlgebra::Vector<T> force, T how_long) {
		//this is an impulse, so we should also include mass but I'm not there yet
		velocity += force.Scale(how_long / mass);
	}

	void ScaleVelocity(const LinearAlgebra::Vector<T>& change) {
		velocity *= change;
	}

	void Move() {
		model->Translate(velocity);
	}
};
