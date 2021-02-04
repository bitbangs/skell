#pragma once
#include <memory>
#include <type_traits>
#include <vector>
#include "FreeBody.hpp"

//this will take an initial position of each body, so we don't need to also have the model for collisions,
//just need to advance that initial by the velocity and a tick; that means we have location being stored
//in 2 places. yea that's repetitive but is it worth it to?  it will keep the model's properties encapsulated
//but we run the risk of these not being in sync...or is that really minimal.  Is there anything else that'd change
//a location after it is spawned?

//looking at model, it looks like it also operates on just changes.  really, storing xx,yy,zz is only helpful when we do
//a TranslateTo call. Can we get rid of this? It is currently only used in the Fire call to entity when we need to shoot
//from the player's current location....oh, but Entity has a FreeBody!!  So if we make TranslateTo a method defined in
//FreeBody, then FreeBody is the only class which knows absolute position.  It will still have to be passed into Model at
//initialization though so am I really helping anything here?  Or just setting myself up to pass one initial state to Model
//and a different one to FreeBody?  Do I need a factory or builder to make sure a Model and FreeBody are initialized in sync?
//let's think about going down that route later...maybe when we construct an Entity it can take care of registering the FreeBody
//with the Collider class, syncing the Model and FreeBody, and any other future stuff which needs to be in step.  And then main.cpp
//doesn't need to give as much of a shit about each class.

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
class Collider {
private:
	std::vector<std::shared_ptr<FreeBody<T>>> bodies;

public:
	Collider() {}
	Collider(std::vector<std::shared_ptr<FreeBody<T>>> bodies) :
		bodies(bodies)
	{}

	void Add(std::shared_ptr<FreeBody<T>> add_me) {
		bodies.push_back(add_me);
	}
	void CheckCollisions() {
		//loop through all freebodies...compare to all others and don't be redundant...
		//or should this be smarter and only compare bodies which are "close" to each other
		//if we store the relationships between bdies as a weighted graph, then we can only
		//look into weights of a certain threshold to decide whether to run collision calculation...
		//but always update the weights.

		//updating the weights should be done based on position and velocity...but doesn't size also matter?
		//up to now, we haven't really dealt with that other than a sort of hacky way of doing it.  might keep that
		//up and come back to it later.  So the model of all FreeBodies will start as a cube of unit side length.
		//position is the center (I think...or is it a corner?)
		//and when a Model is scaled, a FreeBody must also be.  So scaling needs to happen via the entity I guess.
		//it could be factored out as an initialization param and that way the Entity constructor can pass it to both
		//the Model and the FreeBody.
		//However, I'm not scaling a model yet and don't know when I would need to.  If so, the Model can just not keep
		//track of its current scale...just deal with deltas.

		//let's just do this the naive way first and compare everyone to each other
		for (auto ii = 0; ii < bodies.size() - 1; ++ii) {
			for (auto jj = ii + 1; jj < bodies.size(); ++jj) {
				bodies[ii]->CollidesWith(bodies[jj]);
			}
		}
	}
};