#pragma once
#include <cmath>
#include <type_traits>
#include <LinearAlgebra/Matrix.hpp>
#include <LinearAlgebra/Vector.hpp>

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
class Model {
private:
	LinearAlgebra::Matrix<T> model;
	LinearAlgebra::Matrix<T> view;
	LinearAlgebra::Matrix<T> projection;
	LinearAlgebra::Matrix<T> mvp;
	T xx, yy, zz;
	T sx, sy, sz;

public:
	Model() = delete;
	Model(T aspect_ratio) :
		model(LinearAlgebra::Matrix<T>(4, 4, {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		})),
		view(LinearAlgebra::Matrix<T>(4, 4, {
			+1.0f, +0.0f, +0.0f, +0.0f,
			+0.0f, +1.0f, +0.0f, +0.0f,
			+0.0f, +0.0f, +1.0f, +0.0f,
			+0.0f, +0.0f, +10.0f, +1.0f //our position
		})),
		projection(LinearAlgebra::Matrix<T>(4, 4, {
			+1.0f / (+aspect_ratio * std::tanf(3.14159f / 6.0f)), +0.0f, +0.0f, +0.0f,
			0.0f, +1.0f / std::tanf(3.14159f / 6.0f), +0.0f, +0.0f,
			+0.0f, +0.0f, (-1.0f - 100.0f) / (1.0f - 100.0f), +1.0f,
			+0.0f, +0.0f, (+2.0f * 100.0f * 1.0f) / (1.0f - 100.0f), +0.0f
		})),
		mvp(projection * view * model),
		xx(0), yy(0), zz(0),
		sx(1), sy(1), sz(1)
	{}

	Model(T aspect_ratio, T xx, T yy, T zz) :
		model(LinearAlgebra::Matrix<T>(4, 4, {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			xx, yy, zz, 1
		})),
		view(LinearAlgebra::Matrix<T>(4, 4, {
			+1.0f, +0.0f, +0.0f, +0.0f,
			+0.0f, +1.0f, +0.0f, +0.0f,
			+0.0f, +0.0f, +1.0f, +0.0f,
			+0.0f, +0.0f, +10.0f, +1.0f //our position
		})),
		projection(LinearAlgebra::Matrix<T>(4, 4, {
			+1.0f / (+aspect_ratio * std::tanf(3.14159f / 6.0f)), +0.0f, +0.0f, +0.0f,
			0.0f, +1.0f / std::tanf(3.14159f / 6.0f), +0.0f, +0.0f,
			+0.0f, +0.0f, (-1.0f - 100.0f) / (1.0f - 100.0f), +1.0f,
			+0.0f, +0.0f, (+2.0f * 100.0f * 1.0f) / (1.0f - 100.0f), +0.0f
		})),
		mvp(projection * view * model),
		xx(xx), yy(yy), zz(zz),
		sx(1), sy(1), sz(1)
	{}

	void Translate(LinearAlgebra::Vector<T> dt) {
		model *= LinearAlgebra::Matrix<T>(4, 4, {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			dt[0], dt[1], dt[2], 1
			});
		xx += dt[0];
		yy += dt[1];
		zz += dt[2];
	}

	void TranslateTo(const Model<T>& here) {
		Translate(LinearAlgebra::Vector<T>({ here.xx, here.yy, here.zz }) -
			LinearAlgebra::Vector<T>({ xx, yy, zz }));
	}

	void MoveToward(const Model<T>& dest, const T step) {
		auto toward = LinearAlgebra::Vector<T>({ dest.xx, dest.yy, dest.zz }) -
			LinearAlgebra::Vector<T>({xx, yy, zz}); // dest.GetCentroid() - GetCentroid();
		auto norm_toward = toward.Normalize();
		norm_toward.Scale(step);
		Translate(norm_toward);
	}

	void Scale(T dx, T dy, T dz) {
		model *= LinearAlgebra::Matrix<T>(4, 4, {
			dx, 0, 0, 0,
			0, dy, 0, 0,
			0, 0, dz, 0,
			0, 0, 0, 1
			});
		sx *= dx;
		sy *= dy;
		sz *= dz;
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
	bool IsIntersecting(const Model<T>& other) const {
		if ((other.xx + other.sx >= xx && other.xx <= xx + sx)) {
			if (other.yy + other.sy >= yy && other.yy <= yy + sy) {
				return true;
			}
		}
		return false;
	}

	const T* GetMVP() {
		mvp = LinearAlgebra::Matrix<T>(projection * view * model);
		return mvp.GetPointerToData();
	}
};
