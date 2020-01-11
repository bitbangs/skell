#pragma once
#include <LinearAlgebra/Matrix.hpp>

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

	T Getxx() const {
		return xx;
	}
	T Getyy() const {
		return yy;
	}
	T Getzz() const {
		return zz;
	}
	T Getsx() const {
		return sx;
	}
	T Getsy() const {
		return sy;
	}
	T Getsz() const {
		return sz;
	}

	const T* GetPointerToData() const { //not a fan of doing this...
		return model.GetPointerToData();
	}
};