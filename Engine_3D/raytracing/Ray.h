// Ray.h
//

#ifndef _RAY_H_
#define _RAY_H_

#include "../math3d/Vert3D.h"
#include "../common/MultiLink.h"

#define MAX_VERTS_LINK 2
typedef struct Verts Verts;
struct Verts {
	__SUPER(MultiLinkElement, Verts, NULL);
	Verts * _prev[MAX_VERTS_LINK];
	Verts * _next[MAX_VERTS_LINK];

	/////////////////////////////////////////
	EFTYPE trans;
	DWORD color;
	Vert3D v;
	Vert3D v_3;
	Vert3D v_n;
	Vert3D n_r;
	void * obj;

	//ray tracing type
	//0: normal(stop rendering)
	//1: reflection
	//2: transparent
	INT type;
};
_PLATFORM Verts* _Verts(Verts* that);

///////////////////////////////////
typedef struct VertsPool VertsPool;
struct VertsPool {
	__SUPER(ElementPool, VertsPool, Verts);
};
_PLATFORM Verts * VertsPool_at(VertsPool * that, int index);
_PLATFORM void _VertsPool(VertsPool * that, Verts * pool, UMAP * map, int size);

#define MAX_VERTS 100
#define MAP_VERTS GET_MAP_SIZE(MAX_VERTS)
typedef struct VertsPoolImp VertsPoolImp;
struct VertsPoolImp {
	Verts pool[MAX_VERTS];
	UMAP map[MAP_VERTS];

	VertsPool vertsPool;
};
_PLATFORM VertsPoolImp * _VertsPoolImp(VertsPoolImp * that);

typedef struct VertsMan VertsMan;
struct VertsMan {
	__SUPER(MultiLinkBase, VertsMan, Verts);

	VertsPool * vertsPool;

	////////////////////////////
	void(*clearLink)(VertsMan * that);
	////////////////////////////
};
_PLATFORM void VertsMan_clearlink(VertsMan * that);
_PLATFORM VertsMan * _VertsMan(VertsMan * that, int index, VertsPoolImp * poolImp);
/////////////////////////////////////

typedef struct Ray Ray;
struct Ray {
	_PLATFORM Ray() :
		type(0) {
	}
	_PLATFORM Ray(const Vert3D& o, const Vert3D& d) :
		type(0) {
		this->set(o, d);
	}
	_PLATFORM ~Ray() {
	}
	_PLATFORM void set(const Vert3D& o, const Vert3D& d) {
		original.set(o);
		direction.set(d);
	}
	Vert3D original;
	Vert3D direction;
	Vert3D point;
	//ray type:
	//0: normal
	//1: reflection
	//2: refraction
	//3: shadow test ray
	INT type;

	void * obj;

	DWORD color;
	EFTYPE f;

	//r(t) = o + t * d, t >= 0
	_PLATFORM Vert3D& getPoint(float t) {
		point.set(direction);
		point * t;
		point + original;

		return point;
	}
	//r(t) = o + t * d, t >= 0
	_PLATFORM Vert3D& getPoint(float t, Vert3D& point) {
		point.set(direction);
		point * t;
		point + original;

		return point;
	}
};


_PLATFORM Verts* _Verts(Verts* that) {
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_VERTS_LINK);

	/////////////////////////////
	that->type = 0;
	/////////////////////////////
	return that;
}
_PLATFORM void _VertsPool(VertsPool * that, Verts * pool, UMAP * map, int size) {
	_ElementPool(&that->super, (MultiLinkElement*)pool, map, size);

	that->at = VertsPool_at;
}
_PLATFORM Verts * VertsPool_at(VertsPool * that, int index) {
	return &that->pool[index];
}

_PLATFORM VertsPoolImp * _VertsPoolImp(VertsPoolImp * that) {
	for (int i = 0; i < MAX_VERTS; i++) {
		_Verts(&that->pool[i]);
	}
	_VertsPool(&that->vertsPool, that->pool, that->map, MAX_VERTS);

	return that;
}
_PLATFORM void VertsMan_clearlink(VertsMan * that) {
	if (that->link) {
		Verts * temp = that->link;
		do {
			if (that->removeLink(that, temp) == NULL) {
				break;
			}
			if (!temp->free(temp)) {
				that->vertsPool->back(that->vertsPool, temp);
			}

			temp = that->link;
		} while (temp);
	}
}
_PLATFORM VertsMan * _VertsMan(VertsMan * that, int index, VertsPoolImp * poolImp) {
	_MultiLinkBase(&that->super, index);

	that->vertsPool = &poolImp->vertsPool;

	///////////////////////////////////////
	that->clearLink = VertsMan_clearlink;
	///////////////////////////////////////

	return that;
}

/////////////////////////////////////////////////////////////
_PLATFORM double random_double() {
	// Returns a random real in [0,1).
	return rand() / (RAND_MAX + 1.0);
}

_PLATFORM double random_double(double min, double max) {
	// Returns a random real in [min,max).
	return min + (max - min)*random_double();
}
_PLATFORM double clamp(double x, double min, double max) {
	if (x < min) return min;
	if (x > max) return max;
	return x;
}
_PLATFORM void random(Vert3D& v) {
	v.set(random_double(), random_double(), random_double());
}

_PLATFORM void random(Vert3D& v, double min, double max) {
	v.set(random_double(min, max), random_double(min, max), random_double(min, max));
}
_PLATFORM void random_in_unit_sphere(Vert3D& v) {
	while (true) {
		random(v, -1, 1);
		if ((v ^ v) >= 1) continue;
		return;
	}
}
_PLATFORM void random_unit_vector(Vert3D& v) {
	auto a = random_double(0, 2 * EP_PI);
	auto z = random_double(-1, 1);
	auto r = sqrt(1 - z * z);
	v.set(r*cos(a), r*sin(a), z);
}
_PLATFORM void random_in_hemisphere(Vert3D& v, Vert3D& normal) {
	random_in_unit_sphere(v);
	if ((v ^ normal) > 0.0) // In the same hemisphere as the normal
		NULL;
	else
		v.negative();
}
_PLATFORM void random_in_unit_disk(Vert3D& v) {
	while (true) {
		v.set(random_double(-1, 1), random_double(-1, 1), 0);
		if ((v ^ v) >= 1) continue;
		return;
	}
}
_PLATFORM int random_int(int min, int max) {
	// Returns a random integer in [min,max].
	return static_cast<int>(random_double(min, max + 1));
}
_PLATFORM void reflect(const Vert3D& v, const Vert3D& n, Vert3D& r) {
	r.set(n);
	r * (2 * (v ^ n));
	r.negative() + v;
}
_PLATFORM void refract(const Vert3D& uv, const Vert3D& n, double etai_over_etat, Vert3D& r) {
	Vert3D v;
	v.set(uv).negative();
	auto cos_theta = v ^ n;
	((r.set(n) * cos_theta) + uv) * etai_over_etat;
	v.set(n) * (-sqrt(fabs(1.0 - (r ^ r))));
	r + v;
}
_PLATFORM double degrees_to_radians(double degrees) {
	return degrees * EP_PI / 180.0;
}
/////////////////////////////////////////////////////////////
#endif