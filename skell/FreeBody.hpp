#pragma once
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <LinearAlgebra/Vector.hpp>

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
class FreeBody
{
private:
	LinearAlgebra::Vector<T> velocity;
	LinearAlgebra::Vector<T> position;
	T mass;

public:
	FreeBody() = delete;
	FreeBody(
		LinearAlgebra::Vector<T> velocity,
		LinearAlgebra::Vector<T> position,
		T mass
	) :
		velocity(std::move(velocity)),
		position(std::move(position)),
		mass(mass)
	{}

	void Translate(const LinearAlgebra::Vector<T>& dt) {
		position += dt;
	}

	void ApplyImpulse(LinearAlgebra::Vector<T> force, T how_long) {
		velocity += force.Scale(how_long / mass);
	} //we're not yet updating position...so when Entity calls GetPosition to update its Model, it will
	//always get the intial position.  do we Translate with velocity and how_long right here?
	//if so, we're now out of sync with the Model...which also needs to Translate, not just
	//accept an absolute position.  damn...

	//maybe if we take position out of FreeBody we can put it in Entity and FreeBodies can be compared with each
	//other only after they're within a certain distance, determined by a higher class.  is that right?  or does it just
	//sound right? <-- this was the wrong way to go.

	//instead, the current design might work if instead of ApplyImpulse, we actually rely on the collision system...which
	//of course doesn't yet exist...but if we can create forces by spawning FreeBodys and calculating collisions, then
	//...idk this seems weird but might work.  might also be very expensive.

	//for now, let's figure out how to access the position property of FreeBody from the Entity class
	//this is really ugly...and is it even safe?  is it always safe?
	LinearAlgebra::Vector<T> GetPosition() const {
		return { position[0], position[1], position[2] };
	}
	LinearAlgebra::Vector<T> GetVelocity() const {
		return { velocity[0], velocity[1], velocity[2] };
	}

	void Move() {
		Translate(velocity);
	}

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
	*/
};