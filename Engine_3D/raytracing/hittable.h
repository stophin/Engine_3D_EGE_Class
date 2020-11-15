#ifndef HITTABLE_H
#define HITTABLE_H

#include "Ray.h"
#include "aabb.h"

class material;

struct hit_record {
	Vert3D p;
	Vert3D normal;
	double t;
	bool front_face;
	material * material;
	DWORD color;
	bool use_color;

	inline void set_face_normal(Ray& r, Vert3D& outward_normal) {
		front_face = (r.direction ^ outward_normal) < 0;
		front_face ? normal.set(outward_normal) : normal.set(outward_normal).negative();
	}
};

class hittable {
public:
	virtual bool hit(Ray& r, double t_min, double t_max, hit_record& rec) = 0;
	virtual bool bounding_box(double t0, double t1, aabb& output_box) = 0;
};

/////////////////////////////////////////////////////
#define MAX_HITTABLE_MAX 1000
#define MAX_HITTABLE_LINK 3 + MAX_HITTABLE_MAX
typedef struct Hittable Hittable;
struct Hittable {
	__SUPER(MultiLinkElement, Hittable, NULL);
	Hittable * _prev[MAX_HITTABLE_LINK];
	Hittable * _next[MAX_HITTABLE_LINK];

	/////////////////////////////////////
	hittable * obj;
	/////////////////////////////////////
};
_PLATFORM Hittable * _Hittable(Hittable * that, hittable * o) {
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_HITTABLE_LINK);

	/////////////////////////////////////
	that->obj = o;
	/////////////////////////////////////

	return that;
}

/////////////////////////////////////
typedef struct HittablePool HittablePool;
struct HittablePool {
	__SUPER(ElementPool, HittablePool, Hittable);
};
_PLATFORM Hittable * HittablePool_at(HittablePool * that, int index) {
	return &that->pool[index];
}
_PLATFORM void _HittablePool(HittablePool * that, Hittable * pool, UMAP * map, int size) {
	_ElementPool(&that->super, (MultiLinkElement*)pool, map, size);

	that->at = HittablePool_at;
}

#define MAX_HITTABLE	1000
#define MAP_HITTABLE	GET_MAP_SIZE(MAX_HITTABLE)
typedef struct HittablePoolImp HittablePoolImp;
struct HittablePoolImp {
	Hittable pool[MAX_HITTABLE];
	UMAP map[MAP_HITTABLE];

	HittablePool hittablePool;
};
_PLATFORM HittablePoolImp * _HittablePoolImp(HittablePoolImp *that) {

	for (int i = 0; i < MAX_HITTABLE; i++) {
		_Hittable(&that->pool[i], NULL);
	}
	_HittablePool(&that->hittablePool, that->pool, that->map, MAX_HITTABLE);

	return that;
}

typedef bool(*HittableMan_Comp)(Hittable* a, Hittable* b);
typedef struct HittableMan HittableMan;
struct HittableMan {
	__SUPER(MultiLinkBase, HittableMan, Hittable);

	HittablePool * hittablePool;
	HittablePoolImp * hittablePoolImp;

	////////////////////////////
	void(*clearLink)(HittableMan * that);
	void(*sort)(HittableMan * that, int start, int end, HittableMan_Comp fun);
	////////////////////////////
};
_PLATFORM void HittableMan_clearlink(HittableMan * that) {
	if (that->link) {
		Hittable * temp = that->link;
		do {
			if (that->removeLink(that, temp) == NULL) {
				break;
			}
			if (!temp->free(temp)) {
				that->hittablePool->back(that->hittablePool, temp);
			}

			temp = that->link;
		} while (temp);
	}
}
_PLATFORM void HittableMan_sort(HittableMan * that, int start, int end, HittableMan_Comp fun) {
	end = end - 1;
	if (start < 0) {
		return;
	}
	if (end >= that->linkcount) {
		return;
	}
	if (start <= end) {
		return;
	}
}
_PLATFORM HittableMan * _HittableMan(HittableMan * that, int index, HittablePoolImp * poolImp) {
	_MultiLinkBase(&that->super, index);

	that->hittablePool = &poolImp->hittablePool;
	that->hittablePoolImp = poolImp;

	///////////////////////////////////////
	that->clearLink = HittableMan_clearlink;
	that->sort = HittableMan_sort;
	///////////////////////////////////////

	return that;
}
/////////////////////////////////////

#endif