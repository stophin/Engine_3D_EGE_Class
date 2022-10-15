// Camera3D.h
// Camera for 3D
//

#ifndef _CAMERA3D_H_
#define _CAMERA3D_H_

#include "../math3d/Matrix3D.h"

#define MAX_CAM_LINK	1
typedef struct Camera3D Camera3D;
struct Camera3D {
	__SUPER(MultiLinkElement, Camera3D, NULL);
	Camera3D * _prev[MAX_CAM_LINK];
	Camera3D * _next[MAX_CAM_LINK];

	////////////////////////////////
#define Camera3D_Define(T)\
	EFTYPE znear; \
	EFTYPE zfar; \
	EFTYPE width; \
	EFTYPE height; \
	\
	EFTYPE tanwidth; \
	EFTYPE tanheight; \
	\
	Vert3D lb; \
	Vert3D rt; \
	Vert3D lookat; \
	\
	INT type;\
	INT cam_type;\
	\
	Mat3D proj; \
	Mat3D proj_1; \
	\
	Matrix3D _M; \
	Mat3D M; \
	Mat3D M_1; \
	\
	union {\
	\
	Vert3D g; \
	struct {\
	\
	EFTYPE offset_w; \
	EFTYPE offset_h; \
	EFTYPE scale_w; \
	EFTYPE scale_h; \
	}; \
	}; \
	void (*set)(T * that, EFTYPE width, EFTYPE height, EFTYPE znear, EFTYPE zfar, EFTYPE angle_width, EFTYPE angle_height);\
	void (*initialize)(T * that, int type); \
	void (*project)(T* that, Vert3D& v); \
	int (*normalize)(T* that, Vert3D& v); \
	int (*normalize_cut)(T* that, VObj& v, const VObj& v0, const VObj& v1); \
	int (*anti_normalize)(T * that, Vert3D& v, EFTYPE z); \
	void (*setRange)(T * that, EFTYPE o_w, EFTYPE o_h, EFTYPE s_w, EFTYPE s_h); \
	T& (*_move)(T* that, EFTYPE dx, EFTYPE dy, EFTYPE dz); \
	T& (*_rotate)(T* that, EFTYPE ax, EFTYPE ay, EFTYPE az);
	////////////////////////////////
#define Define_Camera3D(T)  Camera3D_Define(T)
	Define_Camera3D(Camera3D)

};
#define Camera3D_Function(T, D, P)

#define T Camera3D
#include "Camera3D_funcs.h"
#undef T

#define Function_Camera3D(T, D, P) Camera3D_Function(T, D, P)
Function_Camera3D(Camera3D, PLATFORM, _)
_PLATFORM Camera3D* _Camera3D(Camera3D* that, EFTYPE width, EFTYPE height, EFTYPE znear, EFTYPE zfar, EFTYPE angle_width, EFTYPE angle_height) {
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_CAM_LINK);

	////////////////////////////////////////////
#define Camera3D_Creator(T, P)\
	that->initialize = Camera3D_initialize ##P; \
	that->normalize = Camera3D_normalize ##P; \
	that->normalize_cut = Camera3D_normalize_cut ##P; \
	that->anti_normalize = Camera3D_anti_normalize ##P; \
	that->setRange = Camera3D_setRange ##P; \
	that->_move = Camera3D_move ##P; \
	that->_rotate = Camera3D_rotate ##P; \
	that->set = Camera3D_set ##P; \
	that->project = Camera3D_project ##P;\
	\
	that->_M.M = &that->M; \
	that->_M.M_1 = &that->M_1; \
	that->_M.mode = 0; \
	that->_M.Init(); \
	\
	that->proj.Init(); \
	that->proj_1.Init();\
	\
	that->lookat.set(0, 0, -1, 1); \
	that->type = 0;\
	that->cam_type = 0;\
	/////////////////////////////////////////////
	Camera3D_Creator(Camera3D, _)

	that->set(that, width, height, znear, zfar, angle_width, angle_height);
	return that;
}

/////////////////////////////////////
typedef struct Cam3D Cam3D;
struct Cam3D {
	__SUPER(MultiLinkElement, Cam3D, NULL);
	Cam3D * _prev[MAX_CAM_LINK];
	Cam3D * _next[MAX_CAM_LINK];

	Camera3D_Define(Cam3D)
};
Camera3D_Function(Cam3D, PLATFORM, __)

#define T Cam3D
#include "Camera3D_funcs.h"
#undef T

_PLATFORM int Camera3D_normalizeEx(Cam3D* that, Vert3D& v) {
	if (that->lookat.z > 0 && (EP_GTZERO(v.z - that->zfar) || EP_GTZERO(v.z - that->znear))) {
		return -2;
	}
	else if (that->lookat.z < 0 && (EP_GTZERO(v.z + that->znear) || EP_LTZERO(v.z + that->zfar))) {
		return -3;
	}
	EFTYPE w = that->tanwidth * v.z, h = that->tanheight * v.z;
	if (w < 0) w = -w;
	if (h < 0) h = -h;
	if (v.x > w || v.x < -w) {
		return -4;
	}
	else if (v.y > h || v.y < -h) {
		return -5;
	}
	if (EP_ISZERO(v.z)) {
		return -1;
	}
	else if (!EP_ISZERO(v.w - EP_MAX)) {
		that->project(that, v);
	}
	return 0;
}
_PLATFORM Cam3D * _Cam3D(Cam3D * that) {
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_CAM_LINK);

	Camera3D_Creator(Cam3D, _)

		that->normalize = Camera3D_normalizeEx;
		return that;
}
//Camera3D_Function(Cam3D, _inline, ___)
//Cam3D * _Cam3DEx(Cam3D * that) {
//	that->prev = that->_prev;
//	that->next = that->_next;
//	_MultiLinkElementEx(&that->super, MAX_CAM_LINK);
//
//	Camera3D_Creator(Cam3D, ___)
//
//	return that;
//}
typedef struct CamPool CamPool;
struct CamPool {
	__SUPER(ElementPool, CamPool, Cam3D);
};
_PLATFORM Cam3D * CamPool_at(CamPool * that, int index) {
	return &that->pool[index];
}
//Cam3D * CamPool_atEx(CamPool * that, int index) {
//	return &that->pool[index];
//}
_PLATFORM void _CamPool(CamPool * that, Cam3D * pool, UMAP * map, int size){
	_ElementPool(&that->super, (MultiLinkElement*)pool, map, size);

	that->at = CamPool_at;
}
//void _CamPoolEx(CamPool * that, Cam3D * pool, UMAP * map, int size){
//	_ElementPoolEx(&that->super, (MultiLinkElement*)pool, map, size);
//
//	that->at = CamPool_atEx;
//}

#define MAX_CAM	100
#define MAP_CAM	GET_MAP_SIZE(MAX_CAM)
typedef struct CamPoolImp CamPoolImp;
struct CamPoolImp {
	Cam3D pool[MAX_CAM];
	UMAP map[MAX_CAM];

	CamPool camPool;
};
_PLATFORM CamPoolImp * _CamPoolImp(CamPoolImp * that) {

	for (int i = 0; i < MAX_CAM; i++) {
		_Cam3D(&that->pool[i]);
	}
	_CamPool(&that->camPool, that->pool, that->map, MAX_CAM);

	return that;
}
typedef struct CamMan CamMan;
struct CamMan{
	__SUPER(MultiLinkBase, CamMan, Cam3D);

	CamPool * camPool;

	////////////////////////////
	void (*clearLink)(CamMan * that);
	////////////////////////////
};
_PLATFORM void CamMan_clearlink(CamMan * that){
	if (that->link) {
		Cam3D * temp = that->link;
		do {
			if (that->removeLink(that, temp) == NULL) {
				break;
			}
			if (!temp->free(temp)) {
				that->camPool->back(that->camPool, temp);
			}

			temp = that->link;
		} while (temp);
	}
}
//void CamMan_clearlinkEx(CamMan * that){
//	Cam3D * o = that->link;
//	Cam3D * n = NULL;
//	if (o) {
//		do {
//			n = that->next(that, o);
//
//			if (that->removeLink(that, o)) {
//				if (!o->free(o)) {
//					that->camPool->back(that->camPool, o);
//				}
//			}
//
//			o = n;
//		} while (o && o != that->link);
//	}
//}
_PLATFORM CamMan * _CamMan(CamMan * that, int index, CamPoolImp * poolImp){
	_MultiLinkBase(&that->super, index);

	that->camPool = &poolImp->camPool;

	///////////////////////////////////////
	that->clearLink = CamMan_clearlink;
	///////////////////////////////////////

	return that;
}
//CamMan * _CamManEx(CamMan * that, int index, CamPoolImp * poolImp){
//	_MultiLinkBaseEx(&that->super, index);
//
//	that->camPool = &poolImp->camPool;
//
//	for (int i = 0; i < MAX_CAM; i++) {
//		_Cam3DEx(&poolImp->pool[i]);
//	}
//	_CamPoolEx(that->camPool, poolImp->pool, poolImp->map, MAX_CAM);
//
//	///////////////////////////////////////
//	that->clearLink = CamMan_clearlinkEx;
//	///////////////////////////////////////
//
//	return that;
//}
/////////////////////////////////////


#endif