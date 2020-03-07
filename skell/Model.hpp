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
		sx(0), sy(0), sz(0)
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

	void TranslateTo(const Model<T>& here) {
		auto dest = LinearAlgebra::Vector<T>({ here.xx, here.yy, here.zz }) -
			LinearAlgebra::Vector<T>({ xx, yy, zz });
		Translate(dest[0], dest[1], dest[2]);
	}

	void MoveToward(const Model<T>& dest, const T step) {
		auto toward = LinearAlgebra::Vector<T>({ dest.xx, dest.yy, dest.zz }) -
			LinearAlgebra::Vector<T>({xx, yy, zz}); // dest.GetCentroid() - GetCentroid();
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

	//to make model only about mvmt and location, intersection can be put in a "frame" class
	//and it will call GetCentroid and GetScale (dne yet) in order to determing the bounding box
	//frame then can have a wireframe mesh for debug
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
