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

	bool IsIntersecting(const Entity<T>& other) const {
		//this really isn't good enough anymore because we need to know which face of the bounding box we
		//bump into. this will allow us to bounce off walls better
		return model->IsIntersecting(*(other.model));
	}

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

	bool Collide(Entity<T>& other) {
		if (this->IsIntersecting(other)) {
			if (other.velocity.Sum() == (T)0) {
				//ApplyForce(velocity.Scale(-1.5f), 0.5f); //real hacky and not working quite right
				//we need a way to collide with the walls which have 0 velocity
				//velocity = velocity.Scale(-1.0f); //invert velocity because the "wall" has relatively infinite mass
				//but we can make the intersection better...
				//because angle of collision matters...so we either need to enhance IsIntersecting, or
				//only bother with these calculations when we detect a collision happened, coarsely.
				//...should have something like
				//velocity = velocity.Scale(-1.0f, 1.0f, 1.0f);
				//when we hit a vertical wall
				//velocity = velocity.Scale(1.0f, -1.0f, 1.0f);
				//when we hit a horizontal (back) wall
				//could just write this "hardcoded", but I'd rather make it general so that when breaking bricks we can reuse the same logic
				if (model->IsIntersectingVerticalFace(*(other.model))) {
					velocity *= { -1.0f, 1.0f, 1.0f };
				}
				else {
					velocity *= { 1.0f, -1.0f, 1.0f };
				}
			}
			else {
				other.ApplyForce(velocity.Scale(1.0f), 0.1f); //scale by 1.0f is just a hack right now to get around not having a copy constructor for Vector
				//also, we really need to take into account each entities mass...and I'm not sure where to put that yet
				//passing velocity is problematic...
				//I should really be doing a force calculation up front
			}
			return true;
		}
		return false;
	}

	void Translate(const LinearAlgebra::Vector<T>& dt) {
		model->Translate(dt);
	}

	void TranslateTo(const Entity<T>& other) {
		model->TranslateTo(*(other.model));
	}

	void ApplyForce(LinearAlgebra::Vector<T> force, T how_long) {
		velocity += force.Scale(how_long / mass); //this is going ok, but we I also need to be able to calculate the equal opposite force
		//I'm not sure I can do that after I've just modified the current velocity...
		//unless I can figure out the force up front then just negate it...that sounds correct
		//meaning when entity A collides into entity B, we need to 
	}

	void ScaleVelocity(const LinearAlgebra::Vector<T>& change) {
		velocity *= change;
	}

	void Move() {
		model->Translate(velocity);
	}
};
