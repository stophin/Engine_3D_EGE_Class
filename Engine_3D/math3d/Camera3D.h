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
#define Camera3D_Function(T, D, P) \
_ ##D void Camera3D_initialize ##T ##P(T * that, int type) {\
	that->type = type;\
	if (type == 1) {\
		that->proj.mx.set(2 * that->znear / that->width, 0, -(that->rt.x + that->lb.x) / that->width, 0);\
		that->proj.my.set(0, 2 * that->znear / that->height, -(that->rt.y + that->lb.y) / that->height, 0);\
		that->proj.mz.set(0, 0, that->zfar / (that->zfar - that->znear), -that->zfar * that->znear / (that->zfar - that->znear));\
		that->proj.mw.set(0, 0, 1, 0);\
\
		that->proj_1.mx.set(1 / (2 * that->znear / that->width), 0, -(that->rt.x + that->lb.x) / that->width, 0);\
		that->proj_1.my.set(0, 1 / (2 * that->znear / that->height), -(that->rt.y + that->lb.y) / that->height, 0);\
		that->proj_1.mz.set(0, 0, 1 / (that->zfar / (that->zfar - that->znear)), (-that->zfar * that->znear / (that->zfar - that->znear)) / (that->zfar / (that->zfar - that->znear)));\
		that->proj_1.mw.set(0, 0, 0, 1);\
	}\
	else if (type == 2) {\
		that->proj.mx.set(2 * that->znear / that->width, 0, -(that->rt.x + that->lb.x) / that->width, 0);\
		that->proj.my.set(0, 2 * that->znear / that->height, -(that->rt.y + that->lb.y) / that->height, 0);\
		that->proj.mz.set(0, 0, (that->zfar + that->znear) / (that->zfar - that->znear), -2 * that->zfar * that->znear / (that->zfar - that->znear));\
		that->proj.mw.set(0, 0, 1, 0);\
\
		that->proj_1.mx.set(1 / (2 * that->znear / that->width), 0, -(that->rt.x + that->lb.x) / that->width, 0);\
		that->proj_1.my.set(0, 1 / (2 * that->znear / that->height), -(that->rt.y + that->lb.y) / that->height, 0);\
		that->proj_1.mz.set(0, 0, 1 / ((that->zfar + that->znear) / (that->zfar - that->znear)), -(-2 * that->zfar * that->znear / (that->zfar - that->znear)) / ((that->zfar + that->znear) / (that->zfar - that->znear)));\
		that->proj_1.mw.set(0, 0, 0, 1);\
	}\
	else {\
		that->proj.mx.set(2 / that->width, 0, 0, -(that->rt.x + that->lb.x) / that->width);\
		that->proj.my.set(0, 2 / that->height, 0, -(that->rt.y + that->lb.y) / that->height);\
		that->proj.mz.set(0, 0, 1 / (that->zfar - that->znear), -that->znear / (that->zfar - that->znear));\
		that->proj.mw.set(0, 0, 0, 1);\
	}\
}\
_ ##D void Camera3D_project ##T ##P(T* that, Vert3D& v) {\
	EFTYPE z = 1.0 / v.z;\
	v * that->proj;\
	v * z;\
}\
_ ##D int Camera3D_normalize_cut ##T ##P(T* that, VObj& v, const VObj& v0, const VObj& v1) {\
	if (that->lookat.z > 0) {\
		if (EP_GTZERO(v.z - that->zfar)) {\
			v.z = that->zfar;\
		}\
		else if (EP_GTZERO(v.z - that->znear)) {\
			v.z = that->znear;\
		}\
	}\
	else if (that->lookat.z < 0) {\
		if (EP_GTZERO(v.z + that->znear)) {\
			v.z = - that->znear;\
		}\
		else if (EP_LTZERO(v.z + that->zfar)) {\
			v.z = - that->zfar;\
		}\
	}\
	EFTYPE w = that->tanwidth * v.z, h = that->tanheight * v.z;\
	if (w < 0) w = -w;\
	if (h < 0) h = -h;\
	if (v.x > w) {\
		v.x = w;\
	}\
	else if (v.x < -w) {\
		v.x = -w;\
	}\
	else if (v.y > h) {\
		v.y = h;\
	}\
	else if (v.y < -h) {\
		v.y = -h;\
	}\
	if (EP_ISZERO(v.z)) {\
		return -1;\
	}\
	else if (!EP_ISZERO(v.w - EP_MAX)){\
		that->project(that, v.v_r);\
	}\
	return 0;\
}\
_ ##D int Camera3D_normalize ##T ##P(T* that, Vert3D& v) {\
	if (that->lookat.z > 0 && (EP_GTZERO(v.z - that->zfar) || EP_GTZERO(v.z - that->znear))) {\
		return -2;\
	}\
	else if (that->lookat.z < 0 && (EP_GTZERO(v.z + that->znear) || EP_GTZERO(v.z + that->zfar))) {\
		return -3;\
	}\
	EFTYPE w = that->tanwidth * v.z, h = that->tanheight * v.z;\
	if (w < 0) w = -w;\
	if (h < 0) h = -h;\
	if (v.x > w || v.x < -w) {\
		return -4;\
	}\
	else if (v.y > h || v.y < -h) {\
		return -5;\
	}\
	if (EP_ISZERO(v.z)) {\
		return -1;\
	}\
	else if (!EP_ISZERO(v.w - EP_MAX)) {\
		that->project(that, v);\
	}\
	return 0;\
}\
_ ##D int Camera3D_anti_normalize ##T ##P(T * that, Vert3D& v, EFTYPE z) {\
	v * z;\
	v.w = 1;\
	v * that->proj_1;\
	v.w = 1;\
	return 0;\
}\
_ ##D void Camera3D_setRange ##T ##P(T * that, EFTYPE o_w, EFTYPE o_h, EFTYPE s_w, EFTYPE s_h) {\
	that->g.set(o_w, o_h, s_w, s_h);\
}\
_ ##D T& Camera3D_move ##T ##P(T* that, EFTYPE dx, EFTYPE dy, EFTYPE dz) {\
	that->_M.move(dx, dy, dz);\
\
	return *that;\
}\
_ ##D T& Camera3D_rotate ##T ##P(T* that, EFTYPE ax, EFTYPE ay, EFTYPE az) {\
	that->_M.rotate(ax, ay, az);\
\
	return *that;\
}\
_ ##D void Camera3D_set ##T ##P(T* that, EFTYPE width, EFTYPE height, EFTYPE znear, EFTYPE zfar, EFTYPE angle_width, EFTYPE angle_height) {\
	that->width = width;\
	that->height = height;\
	that->znear = znear;\
	that->zfar = zfar;\
	that->lb.set(-width / 2.0, -height / 2.0, 0, 1);\
	that->rt.set(width / 2.0, height / 2.0, 0, 1);\
	that->tanwidth = tan(angle_width *  EP_PI  / 180.0);\
	that->tanheight = tan(angle_height *  EP_PI  / 180.0);\
\
	that->initialize(that, 2);\
}
#define Function_Camera3D(T, D, P) Camera3D_Function(T, D, P)
Function_Camera3D(Camera3D, PLATFORM, _)
_PLATFORM Camera3D* _Camera3D(Camera3D* that, EFTYPE width, EFTYPE height, EFTYPE znear, EFTYPE zfar, EFTYPE angle_width, EFTYPE angle_height) {
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_CAM_LINK);

	////////////////////////////////////////////
#define Camera3D_Creator(T, P)\
	that->initialize = Camera3D_initialize ##T ##P; \
	that->normalize = Camera3D_normalize ##T ##P; \
	that->normalize_cut = Camera3D_normalize_cut ##T ##P; \
	that->anti_normalize = Camera3D_anti_normalize ##T ##P; \
	that->setRange = Camera3D_setRange ##T ##P; \
	that->_move = Camera3D_move ##T ##P; \
	that->_rotate = Camera3D_rotate ##T ##P; \
	that->set = Camera3D_set ##T ##P; \
	that->project = Camera3D_project ##T ##P;\
	\
	that->_M.M = &that->M; \
	that->_M.M_1 = &that->M_1; \
	that->_M.mode = 0; \
	that->_M.Init(); \
	\
	that->proj.Init(); \
	that->proj_1.Init();\
	\
	that->lookat.set(0, 0, -1, 1); 
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

	Camera3D_Creator(Cam3D, __)

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