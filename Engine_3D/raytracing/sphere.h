#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "Ray.h"

class sphere : public hittable {
public:
	sphere() {}
	void set(Vert3D& cen, double r) {
		center.set(cen);
		radius = r;
	}
	void set(EFTYPE x, EFTYPE y, EFTYPE z, EFTYPE r) {
		center.set(x, y, z);
		radius = r;
	}

	virtual bool hit(
		Ray& r, double t_min, double t_max, hit_record& rec) override {
		Vert3D oc;
		oc.set(r.original)  - center;
		auto a = r.direction ^ r.direction;
		auto half_b = oc ^ r.direction;
		auto c = (oc ^ oc) - radius * radius;
		auto discriminant = half_b * half_b - a * c;

		if (discriminant > 0) {
			auto root = sqrt(discriminant);

			auto temp = (-half_b - root) / a;
			if (temp < t_max && temp > t_min) {
				rec.t = temp;
				r.getPoint(rec.t, rec.p);
				(oc.set(rec.p) - center) / radius;
				rec.set_face_normal(r, oc);
				rec.material = material;
				return true;
			}

			temp = (-half_b + root) / a;
			if (temp < t_max && temp > t_min) {
				rec.t = temp;
				r.getPoint(rec.t, rec.p);
				(oc.set(rec.p) - center) / radius;
				rec.set_face_normal(r, oc);
				rec.material = material;
				return true;
			}
		}

		return false;
	}

	void set_material(material * m) {
		material = m;
	}
	bool bounding_box(double t0, double t1, aabb& output_box) {
		Vert3D v0, v1;
		v0.set(radius, radius, radius);
		v1.set(v0);
		v0.negative() + center;
		v1 + center;
		output_box.set(v0, v1);
		return true;
	}
public:
	Vert3D center;
	double radius;
	material * material;
};


/////////////////////////////////////////////////////
#define MAX_SPHERE_LINK	4
typedef struct Sphere Sphere;
struct Sphere {
	__SUPER(MultiLinkElement, Sphere, NULL);
	Sphere * _prev[MAX_SPHERE_LINK];
	Sphere * _next[MAX_SPHERE_LINK];

	/////////////////////////////////////
	sphere obj;
	/////////////////////////////////////
};
_PLATFORM Sphere * _Sphere(Sphere * that, sphere * o) {
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_SPHERE_LINK);

	/////////////////////////////////////
	/////////////////////////////////////

	return that;
}

/////////////////////////////////////
typedef struct SpherePool SpherePool;
struct SpherePool {
	__SUPER(ElementPool, SpherePool, Sphere);
};
_PLATFORM Sphere * SpherePool_at(SpherePool * that, int index) {
	return &that->pool[index];
}
_PLATFORM void _SpherePool(SpherePool * that, Sphere * pool, UMAP * map, int size) {
	_ElementPool(&that->super, (MultiLinkElement*)pool, map, size);

	that->at = SpherePool_at;
}

#define MAX_SPHERE	1000
#define MAP_SPHERE	GET_MAP_SIZE(MAX_SPHERE)
typedef struct SpherePoolImp SpherePoolImp;
struct SpherePoolImp {
	Sphere pool[MAX_SPHERE];
	UMAP map[MAP_SPHERE];

	SpherePool spherePool;
};
_PLATFORM SpherePoolImp * _SpherePoolImp(SpherePoolImp *that) {

	for (int i = 0; i < MAX_SPHERE; i++) {
		_Sphere(&that->pool[i], NULL);
	}
	_SpherePool(&that->spherePool, that->pool, that->map, MAX_SPHERE);

	return that;
}

typedef struct SphereMan SphereMan;
struct SphereMan {
	__SUPER(MultiLinkBase, SphereMan, Sphere);

	SpherePool * spherePool;

	////////////////////////////
	void(*clearLink)(SphereMan * that);
	////////////////////////////
};
_PLATFORM void SphereMan_clearlink(SphereMan * that) {
	if (that->link) {
		Sphere * temp = that->link;
		do {
			if (that->removeLink(that, temp) == NULL) {
				break;
			}
			if (!temp->free(temp)) {
				that->spherePool->back(that->spherePool, temp);
			}

			temp = that->link;
		} while (temp);
	}
}
_PLATFORM SphereMan * _SphereMan(SphereMan * that, int index, SpherePoolImp * poolImp) {
	_MultiLinkBase(&that->super, index);

	that->spherePool = &poolImp->spherePool;

	///////////////////////////////////////
	that->clearLink = SphereMan_clearlink;
	///////////////////////////////////////

	return that;
}
/////////////////////////////////////

#endif