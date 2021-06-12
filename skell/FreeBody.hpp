#pragma once
#include <initializer_list>
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
	//these accessors are really ugly...and is it even safe?  is it always safe?
	//this is my bootleg version of copy constructing a vector which I don't feel comfortable with at this point...
	//but haven't ran into a bug yet. this needs some review/research/testing
	LinearAlgebra::Vector<T> GetPosition() const {
		return { position[0], position[1], position[2] };
	}
	LinearAlgebra::Vector<T> GetVelocity() const {
		return { velocity[0], velocity[1], velocity[2] };
	}

	void Move() {
		Translate(velocity);
	}

	//if the player bounces between walls, they continue to gain velocity
	//this is a bug that needs to be fixed
	//possibly related...if the player collides with a projectile while moving in roughly the same
	//direction, the projectile sticks to the player.  it can be released by firing another projectile.
	//...kinda cool and maybe a game modifier later, but definitely a bug now

	void CollidesWith(std::shared_ptr<FreeBody<T>> other) {
		//check for a collision assuming each freebody is a unit size box...we should support scaling better but this
		//is just a proof of concept for now
		//I think position is the (front...no z yet) bottom left corner of the box.
		//check to see if they're even intersecting...if not, exit early

		//this intersection isn't quite there yet...need to be smarter about the below calculation
		//which plane is hitting which plane?

		//does trading high mass for low vertex/instance density of walls balance out somewhere?
		//right now we're at high mass and _same_ vertex/instance density relative to the size of the ball

		if (other->position[0] + 1.0f >= position[0] && other->position[0] <= position[0] + 1.0f) {
			if (other->position[1] + 1.0f >= position[1] && other->position[1] <= position[1] + 1.0f) {
				//elastic collisions do not lose energy, we might start with this and then make each collision lose some energy if that feels more real
				//v1_final = v1_initial * ((m1 - m2)/(m1 + m2)) + v2_initial * ((2 * m2)/(m1 + m2))
				//and vice versa...so we might need some temporaries
				//LinearAlgebra::Vector<T> first_term = velocity * ((mass - other->mass) / (mass + other->mass));
				//instead for now we can write this out the long way...this is ugly and isn't the first time you've
				//had to do this in this class (see accessors)

				//not sure how i landed on the below equations, but these are for head-on collisions in 1 dimension.
				//we can't use this and instead need to come up with something smarter.
				//see giancoli page 228
				T scale = ((mass - other->mass) / (mass + other->mass));
				LinearAlgebra::Vector<T> first_term = { velocity[0] * scale, velocity[1] * scale, velocity[2] * scale };
				scale = ((other->mass + other->mass) / (mass + other->mass));
				LinearAlgebra::Vector<T> second_term = { other->velocity[0] * scale, other->velocity[1] * scale, other->velocity[2] * scale };
				velocity = first_term + second_term;

				scale = ((other->mass - mass) / (other->mass + mass));
				LinearAlgebra::Vector<T> other_first_term = { other->velocity[0] * scale, other->velocity[1] * scale, other->velocity[2] * scale };
				scale = ((mass + mass) / (other->mass + mass));
				LinearAlgebra::Vector<T> other_second_term = { velocity[0] * scale, velocity[1] * scale, velocity[2] * scale };
				other->velocity = other_first_term + other_second_term;
			}
		}
	}
};