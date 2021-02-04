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
	std::shared_ptr<Drawer<T>> drawer; //shared because we'll reuse these...maybe this should soon be factored out just like
	//we're factoring out the more interesting work done to free_body into Collider (the "all-knowing" class)...but do we really
	//need another all-knowing?  Maybe it can reduce the "wasteful" context changes in opengl by using something smarter to order
	//draw calls.  let's get to this much later but keep this comment here for now.
	std::unique_ptr<Model<T>> model; //unique because it is needed for drawing and absolute position, but maybe absolute position doesn't need to
	//exist in the model except for at initialization
	std::shared_ptr<FreeBody<T>> free_body; //shared because we want to update these from an "all-knowing" collision class, but need to
	//know who actually owns which free body so it can be used to update models
	GLuint texture_id;
	//LinearAlgebra::Vector<T> position;

	void Translate(const LinearAlgebra::Vector<T>& dt) {
		model->Translate(dt);
		free_body->Translate(dt);
	}

	void TranslateTo(const Entity<T>& other) {
		//this->Translate({other.xx - xx, other.yy - yy, other.zz - zz});
		Translate(other.free_body->GetPosition() - free_body->GetPosition());
	}

public:
	Entity() = delete;
	Entity(std::shared_ptr<Mesh<T>> mesh,
		std::shared_ptr<Drawer<T>> drawer,
		T aspect_ratio, //should all models use the same aspect ratio?
		std::shared_ptr<FreeBody<T>> free_body,
		GLuint texture_id) ://,
		//LinearAlgebra::Vector<T> position) :
		mesh(mesh),
		drawer(drawer),
		model(std::move(std::make_unique<Model<T>>(aspect_ratio))),
		free_body(free_body),
		texture_id(texture_id)//,
		//position(std::move(position))
	{}
	Entity(std::shared_ptr<Mesh<T>> mesh,
		std::shared_ptr<Drawer<T>> drawer,
		std::vector<T> model_args, //can I do better than vector?
		std::shared_ptr<FreeBody<T>> free_body,
		GLuint texture_id) ://,
		//LinearAlgebra::Vector<T> position) :
		mesh(mesh),
		drawer(drawer),
		model(std::make_unique<Model<T>>(model_args[0], model_args[1], model_args[2], model_args[3])),
		free_body(free_body),
		texture_id(texture_id)//,
		//position(std::move(position))
	{}

	void Draw() {
		glBindVertexArray(mesh->GetVao()); //wasteful
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIbo()); //wasteful
		glBindTexture(GL_TEXTURE_2D, texture_id);
		drawer->GetShaderProgram().Use(); //wasteful
		drawer->GetShaderProgram().SetMatrixBuffer("mvp", model->GetMVP()); //wasteful
		glDrawElements(GL_TRIANGLES, mesh->GetNumIndices(), GL_UNSIGNED_INT, 0);
	}

	void ApplyImpulse(LinearAlgebra::Vector<T> force, T how_long) {
		//except that I don't have velocity on Entity anymore....so how am I going to make this change? casll both the model and freebody
		//velocity for an updatE?
		//velocity += force.Scale(how_long / mass); //this is going ok, but we I also need to be able to calculate the equal opposite force
		//I'm not sure I can do that after I've just modified the current velocity...
		//unless I can figure out the force up front then just negate it...that sounds correct
		//meaning when entity A collides into entity B, we need to

		//ok so given how this was done before, I now need an ApplyImpulse in each of the Model and FreeBody...
		//but here is the problem that's been bothering me! --> mass determines how this is going to move//and mass is only there in the freebody
		//so an Entity would have to own the mass...or the FreeBody would have to move the Model, if the FreeBody is to still own the mass, which
		//is probably the right way to go here.  Bummer...unless the builder takes care of this connection.  So for now we kind of
		//hack together the builder and come back to it later.  And it probably will make sense for the FreeBody to talk to the Model
		//but don't forget the Entity needs the model for draw calls.  Idk it feels like stuff is starting to unravel and the usefulness of 
		//Entity is going to fade. Or maybe I'm overthinking it and this is a good interface between main.cpp and each "game thing" it can tell
		//what to do

		//Model class' purpose is really just to keep track of the mvp for draw calls.
		//If we ApplyImpulse to FreeBody, it could modify itself and return the translation vector
		//needed by Model to update the model matrix
		//For now, let's just update the FreeBody and assume that later we'll have an update method on Entity
		//which will take an elapsed time and tell Model the translation given we've kept track of the current velocity
		free_body->ApplyImpulse(std::move(force), how_long);
	}

	//intersection will now need to care about xx vs yy relative to "other"'s bounding box
	//depending on direction vector, only need to check 2 planes on each entity
	//(or one if other component is 0)
	//depending on location of model, only need to check certain other entities
	//this means you need a geometry manager that knows everyone
	//geometry manager needs multiple (how many? ..i first though 2, but it is planes instead...so 4) sorted data structures
	//see who is in both when moving diagonally; your vector tells you which 2 to check...or possibly just 1
	//or better yet...if you trigger a swap in one list, then a swap in the other...there's your intersection plane.
	//if you trigger one, and one of your components (xx/yy) is 0, there's your intersection plane.

	//so what does a model need to provide the other class?
	//^^^the answer may be to add another indirection to a physics Body class, which is starting to look better from main
	//but not sure how it will apply here yet. the direction (did we mean velocity?) vector should live in Body.
	//maybe even keep the below method and only call it for Models which have nearby positions...that "lazy" collision
	//calculation algo is still kind of a nightmare to think of right now
	//bool IsIntersecting(const Entity<T>& other) const {
	//	if ((other.xx + other.model.GetSx() >= xx && other.xx <= xx + model.GetSx())) {
	//		if (other.yy + other.model.GetSy() >= yy && other.yy <= yy + model.GetSy()) {
	//			return true;
	//		}
	//	}
	//	return false;
	//}
	//this is commented out because we should be dealing with intersections between FreeBody classes

	//bool IsIntersectingVerticalFace(const Entity<T>& other) const { //really doesn't work if we start rotating...so this is kinda hacky
	//	//also should this really be in model?
	//	return std::abs(other.xx - xx) > std::abs(other.yy - yy);
	//}
	//this is commented out because it was probably just a band-aid for the early brickbreaker-type code

	void Fire(Entity<T>& projectile) {
		projectile.TranslateTo(*this);
		projectile.Translate({ 0.0f, 1.4f, 0.0f });
	}

	void Move() {
		//before, this called model->Translate(velocity)
		//Now the velocity is only in FreeBody
		//Again, I'm finding reasons to take the model matrix out of Model class although
		//really we're talking about copyin
		model->Translate(free_body->GetVelocity());
		//and we're not updating the free_body yet but definitely need to for when we start
		//comparing them to each other or calling Fire
		free_body->Move();
	}
};
