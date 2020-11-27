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

struct records {
	hit_record rec;
	Ray ray;
	Vert3D color;
};

/////////////////////////////////////////////////////
#define MAX_RECORDS_MAX 1
#define MAX_RECORDS_LINK 1 + MAX_RECORDS_MAX
typedef struct Records Records;
struct Records {
	__SUPER(MultiLinkElement, Records, NULL);
	Records * _prev[MAX_RECORDS_LINK];
	Records * _next[MAX_RECORDS_LINK];

	/////////////////////////////////////
	records obj;
	/////////////////////////////////////
};
_PLATFORM Records * _Records(Records * that, records * o) {
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_RECORDS_LINK);

	/////////////////////////////////////
	//that->obj = o;
	/////////////////////////////////////

	return that;
}

/////////////////////////////////////
typedef struct RecordsPool RecordsPool;
struct RecordsPool {
	__SUPER(ElementPool, RecordsPool, Records);
};
_PLATFORM Records * RecordsPool_at(RecordsPool * that, int index) {
	return &that->pool[index];
}
_PLATFORM void _RecordsPool(RecordsPool * that, Records * pool, UMAP * map, int size) {
	_ElementPool(&that->super, (MultiLinkElement*)pool, map, size);

	that->at = RecordsPool_at;
}

#define MAX_RECORDS	1000
#define MAP_RECORDS	GET_MAP_SIZE(MAX_RECORDS)
typedef struct RecordsPoolImp RecordsPoolImp;
struct RecordsPoolImp {
	Records pool[MAX_RECORDS];
	UMAP map[MAP_RECORDS];

	RecordsPool recordsPool;
};
_PLATFORM RecordsPoolImp * _RecordsPoolImp(RecordsPoolImp *that) {

	for (int i = 0; i < MAX_RECORDS; i++) {
		_Records(&that->pool[i], NULL);
	}
	_RecordsPool(&that->recordsPool, that->pool, that->map, MAX_RECORDS);

	return that;
}

typedef bool(*RecordsMan_Comp)(Records* a, Records* b);
typedef struct RecordsMan RecordsMan;
struct RecordsMan {
	__SUPER(MultiLinkBase, RecordsMan, Records);

	RecordsPool * recordsPool;
	RecordsPoolImp * recordsPoolImp;

	////////////////////////////
	void(*clearLink)(RecordsMan * that);
	Records* (*popLink)(RecordsMan * that);
	void(*sort)(RecordsMan * that, int start, int end, RecordsMan_Comp fun);
	////////////////////////////
};
_PLATFORM Records*  RecordsMan_poplink(RecordsMan * that) {
	if (that->link) {
		Records * temp = that->prev(that, that->link);
		if (temp) {
			if (that->removeLink(that, temp) == NULL) {
				return NULL;
			}
			if (!temp->free(temp)) {
				that->recordsPool->back(that->recordsPool, temp);
			}
			return temp;
		}
	}
	return NULL;
}
_PLATFORM void RecordsMan_clearlink(RecordsMan * that) {
	if (that->link) {
		Records * temp = that->link;
		do {
			if (that->removeLink(that, temp) == NULL) {
				break;
			}
			if (!temp->free(temp)) {
				that->recordsPool->back(that->recordsPool, temp);
			}

			temp = that->link;
		} while (temp);
	}
}
_PLATFORM void RecordsMan_sort(RecordsMan * that, int start, int end, RecordsMan_Comp fun) {
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
_PLATFORM RecordsMan * _RecordsMan(RecordsMan * that, int index, RecordsPoolImp * poolImp) {
	_MultiLinkBase(&that->super, index);

	that->recordsPool = &poolImp->recordsPool;
	that->recordsPoolImp = poolImp;

	///////////////////////////////////////
	that->clearLink = RecordsMan_clearlink;
	that->popLink = RecordsMan_poplink;
	that->sort = RecordsMan_sort;
	///////////////////////////////////////

	return that;
}
/////////////////////////////////////
#endif