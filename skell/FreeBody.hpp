#pragma once
#include <initializer_list>
#include <type_traits>
#include <LinearAlgebra/Vector.hpp>

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
class FreeBody
{
private:
	LinearAlgebra::Vector<T> velocity; //lets soon replace this with a mass
	T mass;

public:
	FreeBody() = delete;
	FreeBody(LinearAlgebra::Vector<T> velocity, T mass) :
		velocity(std::move(velocity)),
		mass(mass)
	{}
	/*
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

	void ApplyForce(LinearAlgebra::Vector<T> force, T how_long) {
		velocity += force.Scale(how_long / mass); //this is going ok, but we I also need to be able to calculate the equal opposite force
		//I'm not sure I can do that after I've just modified the current velocity...
		//unless I can figure out the force up front then just negate it...that sounds correct
		//meaning when entity A collides into entity B, we need to 
	}

	void Move() {
		velocity -= velocity * LinearAlgebra::Vector<T>({ 0.001f, 0.001f, 1.0f });
	}
	*/
};