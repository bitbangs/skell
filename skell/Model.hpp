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
		sx(1), sy(1), sz(1)
	{}

	void Translate(const LinearAlgebra::Vector<T>& dt) {
		model *= LinearAlgebra::Matrix<T>(4, 4, {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			dt[0], dt[1], dt[2], 1
			});
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

	const T* GetMVP() {
		mvp = LinearAlgebra::Matrix<T>(projection * view * model);
		return mvp.GetPointerToData();
	}
};
