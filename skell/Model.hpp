#pragma once
#include <LinearAlgebra/Matrix.hpp>
#include <LinearAlgebra/Vector.hpp>

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
class Model {
private:
	LinearAlgebra::Matrix<T> model;
	T xx, yy, zz;
	T sx, sy, sz;
public:
	Model() :
		model(LinearAlgebra::Matrix<T>(4, 4, {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		})),
		xx(0), yy(0), zz(0),
		sx(1), sy(1), sz(1)
	{}
	Model(std::initializer_list<T> list) :
		model(LinearAlgebra::Matrix<T>(4, 4, list)),
		xx(*(list.begin() + 12)),
		yy(*(list.begin() + 13)),
		zz(*(list.begin() + 14)),
		sx(*(list.begin())),
		sy(*(list.begin() + 5)),
		sz(*(list.begin() + 10))
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

	const T* GetPointerToData() const { //not a fan of doing this...
		return model.GetPointerToData();
	}
};