#ifndef AABB_H
#define AABB_H

#include "Ray.h"

class aabb {
public:
	aabb() {}

	void set(const Vert3D& a, const Vert3D& b) {
		_min.set(a);
		_max.set(b);
	}

	void set(const aabb& ab) {
		_min.set(ab._min);
		_max.set(ab._max);
	}

	bool hit(Ray& r, double tmin, double tmax) {
		for (int a = 0; a < 3; a++) {
			auto t0 = fmin((_min[a] - r.original[a]) / r.direction[a],
				(_max[a] - r.original[a]) / r.direction[a]);
			auto t1 = fmax((_min[a] - r.original[a]) / r.direction[a],
				(_max[a] - r.original[a]) / r.direction[a]);
			tmin = fmax(t0, tmin);
			tmax = fmin(t1, tmax);
			if (tmax <= tmin)
				return false;
		}
		return true;
	}

	bool hit_opti(const Ray& r, double tmin, double tmax) {
		for (int a = 0; a < 3; a++) {
			auto invD = 1.0f / r.direction[a];
			auto t0 = (_min[a] - r.original[a]) * invD;
			auto t1 = (_max[a] - r.original[a]) * invD;
			if (invD < 0.0f) {
				auto t = t0;
				t0 = t1;
				t1 = t;
			}
			tmin = t0 > tmin ? t0 : tmin;
			tmax = t1 < tmax ? t1 : tmax;
			if (tmax <= tmin)
				return false;
		}
		return true;
	}

	Vert3D _min;
	Vert3D _max;
};

inline void surrounding_box(aabb& box0, aabb& box1, aabb& r) {
	Vert3D sma, big;
	sma.set(fmin(box0._min.x, box1._min.x),
		fmin(box0._min.y, box1._min.y),
		fmin(box0._min.z, box1._min.z));

	big.set(fmax(box0._max.x, box1._max.x),
		fmax(box0._max.y, box1._max.y),
		fmax(box0._max.z, box1._max.z));

	r.set(sma, big);
}

#endif