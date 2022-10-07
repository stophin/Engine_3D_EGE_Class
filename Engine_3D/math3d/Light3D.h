// Light3D.h
//

#ifndef _LIGHT3D_H_
#define _LIGHT3D_H_

#include "../math3d/Vert3D.h"
#include "../flatform/EPaint.h"

#define MAX_LGT_LINK	1
typedef struct Light3D Light3D;
struct Light3D {
	__SUPER(MultiLinkElement, Light3D, NULL);
	Light3D * _prev[MAX_LGT_LINK];
	Light3D * _next[MAX_LGT_LINK];

	////////////////////////////////
#define Light3D_Define(T)\
	DWORD color; \
	Vert3D pos; \
	Vert3D pos_r; \
	\
	int mode; \
	\
	Camera3D * cam; \
	Matrix3D _M; \
	Mat3D M; \
	Mat3D M_1; \
	\
	Vert3D n1; \
	Vert3D r; \
	\
	void (*render_normalize)(T * that); \
	T& (*_move)(T* that, EFTYPE dx, EFTYPE dy, EFTYPE dz); \
	T& (*_rotate)(T* that, EFTYPE dx, EFTYPE dy, EFTYPE dz); \
	EFTYPE (*getFactor)(T* that, const Vert3D& n, const Vert3D& n0);
	////////////////////////////////
#define Define_Light3D(T)  Light3D_Define(T)
	Define_Light3D(Light3D)
};
#define Light3D_Function(T, D, P)

#define T Light3D
#include "Light3D_funcs.h"
#undef T

#define Function_Light3D(T, D, P) Light3D_Function(T, D, P)
Function_Light3D(Light3D, PLATFORM, _)
_PLATFORM Light3D * _Light3D(Light3D * that) {
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_LGT_LINK);

	/////////////////////////////////////
#define Light3D_Creator(T, P)\
	that->render_normalize = Light3D_render_normalize ##P; \
	that->_move = Light3D_move ##P; \
	that->_rotate = Light3D_rotate ##P; \
	that->getFactor = Light3D_getFactor ##P; \
	\
	that->_M.M = &that->M; \
	that->_M.M_1 = &that->M_1; \
	that->_M.mode = 1;\
	that->_M.Init(); \
	\
	that->cam = NULL; \
	that->mode = 1; \
	that->pos.init(); \
	that->pos_r.init();
	/////////////////////////////////////
	Light3D_Creator(Light3D, _)
	return that;
}
_PLATFORM static DWORD Light3D_add(DWORD rgba, DWORD rgba_r, EFTYPE s) {
	BYTE r = EGEGET_R(rgba), g = EGEGET_G(rgba), b = EGEGET_B(rgba), a = EGEGET_A(rgba);
	BYTE r_r = EGEGET_R(rgba_r), g_r = EGEGET_G(rgba_r), b_r = EGEGET_B(rgba_r), a_r = EGEGET_A(rgba_r);

	s /= 2;
	r = EP_GT(EP_LT(r * s + r_r * (1 - s), 255), 0);
	g = EP_GT(EP_LT(g * s + g_r * (1 - s), 255), 0);
	b = EP_GT(EP_LT(b * s + b_r * (1 - s), 255), 0);
	a = EP_GT(EP_LT(a * s + a_r * (1 - s), 255), 0);

	return EGERGBA(r, g, b, a);

}
_PLATFORM static DWORD Light3D_multi(DWORD rgba, EFTYPE s) {
	BYTE r = EGEGET_R(rgba), g = EGEGET_G(rgba), b = EGEGET_B(rgba), a = EGEGET_A(rgba);

	r = EP_GT(EP_LT(r * s, 255), 0);
	g = EP_GT(EP_LT(g * s, 255), 0);
	b = EP_GT(EP_LT(b * s, 255), 0);
	a = EP_GT(EP_LT(a * s, 255), 0);

	return EGERGBA(r, g, b, a);

}


/////////////////////////////////////
typedef struct Lgt3D Lgt3D;
struct Lgt3D {
	__SUPER(MultiLinkElement, Lgt3D, NULL);
	Lgt3D * _prev[MAX_LGT_LINK];
	Lgt3D * _next[MAX_LGT_LINK];

	Light3D_Define(Lgt3D)
};
Light3D_Function(Lgt3D, PLATFORM, __)

#define T Lgt3D
#include "Light3D_funcs.h"
#undef T

_PLATFORM Lgt3D * _Lgt3D(Lgt3D * that) {
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_LGT_LINK);

	Light3D_Creator(Lgt3D, _)

	return that;
}
//Light3D_Function(Lgt3D, _inline, ___)
//Lgt3D * _Lgt3DEx(Lgt3D * that) {
//	that->prev = that->_prev;
//	that->next = that->_next;
//	_MultiLinkElementEx(&that->super, MAX_LGT_LINK);
//
//	Light3D_Creator(Lgt3D, ___)
//
//	return that;
//}
typedef struct LgtPool LgtPool;
struct LgtPool {
	__SUPER(ElementPool, LgtPool, Lgt3D);
};
_PLATFORM Lgt3D * LgtPool_at(LgtPool * that, int index) {
	return &that->pool[index];
}
//Lgt3D * LgtPool_atEx(LgtPool * that, int index) {
//	return &that->pool[index];
//}
_PLATFORM void _LgtPool(LgtPool * that, Lgt3D * pool, UMAP * map, int size){
	_ElementPool(&that->super, (MultiLinkElement*)pool, map, size);

	that->at = LgtPool_at;
}
//void _LgtPoolEx(LgtPool * that, Lgt3D * pool, UMAP * map, int size){
//	_ElementPoolEx(&that->super, (MultiLinkElement*)pool, map, size);
//
//	that->at = LgtPool_atEx;
//}

#define MAX_LGT	100
#define MAP_LGT	GET_MAP_SIZE(MAX_LGT)
typedef struct LgtPoolImp LgtPoolImp;
struct LgtPoolImp {
	Lgt3D pool[MAX_LGT];
	UMAP map[MAX_LGT];

	LgtPool lgtPool;
};
_PLATFORM LgtPoolImp * _LgtPoolImp(LgtPoolImp* that) {
	for (int i = 0; i < MAX_LGT; i++) {
		_Lgt3D(&that->pool[i]);
	}
	_LgtPool(&that->lgtPool, that->pool, that->map, MAX_LGT);

	return that;
}
typedef struct LgtMan LgtMan;
struct LgtMan{
	__SUPER(MultiLinkBase, LgtMan, Lgt3D);

	LgtPool * lgtPool;

	////////////////////////////
	void (*clearLink)(LgtMan * that);
	////////////////////////////
};
_PLATFORM void LgtMan_clearlink(LgtMan * that){
	if (that->link) {
		Lgt3D * temp = that->link;
		do {
			if (that->removeLink(that, temp) == NULL) {
				break;
			}
			if (!temp-> free(temp)) {
				that->lgtPool->back(that->lgtPool, temp);
			}

			temp = that->link;
		} while (temp);
	}
}
//void LgtMan_clearlinkEx(LgtMan * that){
//	Lgt3D * o = that->link;
//	Lgt3D * n = NULL;
//	if (o) {
//		do {
//			n = that->next(that, o);
//
//			if (that->removeLink(that, o)) {
//				if (!o->free(o)) {
//					that->lgtPool->back(that->lgtPool, o);
//				}
//			}
//
//			o = n;
//		} while (o && o != that->link);
//	}
//}
_PLATFORM LgtMan * _LgtMan(LgtMan * that, int index, LgtPoolImp * poolImp){
	_MultiLinkBase(&that->super, index);

	that->lgtPool = &poolImp->lgtPool;

	///////////////////////////////////////
	that->clearLink = LgtMan_clearlink;
	///////////////////////////////////////

	return that;
}
//LgtMan * _LgtManEx(LgtMan * that, int index, LgtPoolImp * poolImp){
//	_MultiLinkBaseEx(&that->super, index);
//
//	that->lgtPool = &poolImp->lgtPool;
//
//	for (int i = 0; i < MAX_LGT; i++) {
//		_Lgt3DEx(&poolImp->pool[i]);
//	}
//	_LgtPoolEx(that->lgtPool, poolImp->pool, poolImp->map, MAX_LGT);
//
//	///////////////////////////////////////
//	that->clearLink = LgtMan_clearlinkEx;
//	///////////////////////////////////////
//
//	return that;
//}
/////////////////////////////////////

#endif