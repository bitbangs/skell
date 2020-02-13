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
	T xx, yy, zz;
	T sx, sy, sz;
	T rotz;
public:
	Model() :
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
			+0.0f, +0.0f, +0.5f, +1.0f //our position
			})),
		xx(0), yy(0), zz(0),
		sx(1), sy(1), sz(1),
		rotz(0)
	{}

	Model(std::initializer_list<T> list) :
		model(LinearAlgebra::Matrix<T>(4, 4, list)),
		view(LinearAlgebra::Matrix<T>(4, 4, {
			+1.0f, +0.0f, +0.0f, +0.0f,
			+0.0f, +1.0f, +0.0f, +0.0f,
			+0.0f, +0.0f, +1.0f, +0.0f,
			+0.0f, +0.0f, +0.5f, +1.0f //our position
			})),
		xx(*(list.begin() + 12)),
		yy(*(list.begin() + 13)),
		zz(*(list.begin() + 14)),
		sx(*(list.begin())),
		sy(*(list.begin() + 5)),
		sz(*(list.begin() + 10)),
		rotz(0)
	{}

	void Translate(T dx, T dy, T dz) {
		model *= LinearAlgebra::Matrix<T>(4, 4, {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			dx, dy, dz, 1
			});
		xx += dx;
		yy += dy;
		zz += dz;
	}
	void MoveToward(const Model<T>& dest, const T step) {
		auto toward = dest.GetCentroid() - GetCentroid();
		auto norm_toward = toward.Normalize();
		norm_toward.Scale(step);
		Translate(norm_toward[0], norm_toward[1], norm_toward[2]);
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

	void RotateZ(T dz) {
		view *= LinearAlgebra::Matrix<T>(4, 4, {
			+std::cos(dz), +std::sin(dz), 0, 0,
			-std::sin(dz), +std::cos(dz), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
			});
		rotz += dz;
	}

	bool IsIntersecting(const Model<T>& other) const {
		if ((other.xx + other.sx >= xx && other.xx <= xx + sx)) {
			if (other.yy + other.sy >= yy && other.yy <= yy + sy) {
				return true;
			}
		}
		return false;
	}

	LinearAlgebra::Vector<T> GetCentroid() const {
		LinearAlgebra::Vector<T> centroid({
			xx + (sx / (T)2),
			yy + (sy / (T)2),
			zz + (sz / (T)2)
			});
		return centroid;
	}

	T GetRotation() const {
		return rotz;
	}

	const T* GetPointerToModelData() const { //not a fan of doing this...
		return model.GetPointerToData();
	}
	const T* GetPointerToViewData() const {
		return view.GetPointerToData();
	}
};