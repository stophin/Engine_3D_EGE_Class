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
#endif