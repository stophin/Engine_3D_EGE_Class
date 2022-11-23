// Object3D.h
// Object for 3D
//

#ifndef _OBJECT3D_H_
#define _OBJECT3D_H_

#include "../math3d/Matrix3D.h"

#include "../math3d/Camera3D.h"
#include "../math3d/Light3D.h"
#include "../math3d/Texture3D.h"
#include "../math3d/Texture.h"

#define MAX_OBJ3D_MAX	15000//used for all object
#define MAX_OBJ3D_START 60
#define MAX_OBJ3D_LINK	MAX_OBJ3D_START + 1000 //not exceed thread * thread
#define MAX_OBJ3D_END	6//reserved for normal linklist use
#define MAX_OBJ3D_THREAD	MAX_OBJ3D_MAX - MAX_OBJ3D_LINK - MAX_OBJ3D_END// the left are for thread
typedef struct Object3D Object3D;
struct Object3D {
	__SUPER(MultiLinkElement, Object3D, NULL);
	Object3D * _prev[MAX_OBJ3D_LINK];
	Object3D * _next[MAX_OBJ3D_LINK];

	/////////////////////////////
#define OBject3D_Define(T)\
	void * material; \
	void * octTree; \
	VObjMan indice;\
	VObjMan verts; \
	VObjMan verts_r; \
	VObjMan verts_f; \
	\
	EFTYPE reflection; \
	EFTYPE transparent; \
	\
	DWORD *texture; \
	INT t_w; \
	INT t_h; \
	INT texture_type; \
	EFTYPE u; \
	EFTYPE v; \
	\
	INT backfaceculling;\
	\
	Camera3D * cam; \
	\
	Vert3D center; \
	Vert3D center_w; \
	Vert3D center_r; \
	\
	COLORREF color; \
	COLORREF line; \
	INT debugger;\
	\
	VObj * v0;\
	VObj * v1;\
	INT render_aabb;\
	union {\
		Vert3D aabb[8];\
		struct {\
			Vert3D _aabb_o[2];\
			Vert3D rightBottomFront_O;\
			Vert3D __aabb_o;\
			Vert3D leftTopBack_O;\
			Vert3D ___aabb_o[3];\
		};\
	};\
	union {\
		Vert3D aabb_w[8];\
		struct {\
			Vert3D _aabb[2];\
			Vert3D rightBottomFront;\
			Vert3D __aabb;\
			Vert3D leftTopBack;\
			Vert3D ___aabb[3];\
		};\
	};\
	union {\
		Vert3D aabb_r[8];\
		struct {\
			Vert3D _aabb_r[2];\
			Vert3D rightBottomFront_R;\
			Vert3D __aabb_r;\
			Vert3D leftTopBack_R;\
			Vert3D ___aabb_r[3];\
		};\
	};\
	\
	Matrix3D _M;\
	Mat3D M;\
	Mat3D M_1;\
	Mat3D CM;\
	\
	INT vertex_type;\
	INT normal_type;\
	\
	INT anti; \
	\
	T& (*setTexture)(T* that, TextureLocalManager& tman, INT uID, INT t);\
	T& (*setTextureD)(T* that, TextureManager& tman, INT uID, INT t);\
	T& (*setBackfaceCulling)(T* that, INT bfc); \
	T& (*setUV)(T* that, EFTYPE u, EFTYPE v); \
	T& (*setNormalType)(T* that, INT n); \
	T& (*setVertexType)(T* that, INT type); \
	DWORD (*getTextureActual)(T* that, EFTYPE x, EFTYPE y);\
	DWORD (*getTexture)(T* that, EFTYPE x, EFTYPE y);\
	DWORD (*getTextureColor)(T* that, Vert3D& n0, Vert3D& n1, Vert3D& n2, Vert3D& n3, VObj * v, VObj* v0, VObj* v1, Vert3D* v_n);\
	T& (*setCenter)(T* that, EFTYPE x, EFTYPE y, EFTYPE z); \
	T& (*setLineColor)(T* that, COLORREF line); \
	T& (*setColor)(T* that, COLORREF color); \
	T& (*setColorB)(T* that, BYTE * c); \
	T& (*addIndice)(T* that, EFTYPE x, EFTYPE y, EFTYPE z);\
	T& (*addIndiceN)(T* that, EFTYPE x, EFTYPE y, EFTYPE z, EFTYPE nx, EFTYPE ny, EFTYPE nz);\
	T& (*setIndice)(T* that, INT pv, INT pv0, INT pv1);\
	T& (*setIndiceI)(T* that, INT pv, INT pv0, INT pv1, INT anti_n);\
	T& (*addVertI)(T* that, VObj* v); \
	T& (*addVertN)(T* that, EFTYPE x, EFTYPE y, EFTYPE z, EFTYPE nx, EFTYPE ny, EFTYPE nz); \
	T& (*addVert)(T* that, EFTYPE x, EFTYPE y, EFTYPE z); \
	T& (*addVertA)(T* that, EFTYPE x, EFTYPE y, EFTYPE z, INT anti); \
	T& (*renderAABB)(T* that); \
	void (*refreshAABBW)(T* that);\
	void (*refreshAABBC)(T* that);\
	void (*shaderVertex)(T* that);\
	void (*render_normalize)(T* that, int mode); \
	T& (*_move)(T* that, EFTYPE dx, EFTYPE dy, EFTYPE dz); \
	T& (*_scale)(T* that, EFTYPE sx, EFTYPE sy, EFTYPE sz); \
	T& (*_rotate)(T*that, EFTYPE ax, EFTYPE ay, EFTYPE az); 
	/////////////////////////////////////
#define Define_Object3D(T)  OBject3D_Define(T)
Define_Object3D(Object3D)
};
#define Object3D_Function(T, D, P) 

#define T Object3D
#include "Object3D_funcs.h"
#undef T

#define Function_Object3D(T, D, P) Object3D_Function(T, D, P)
Function_Object3D(Object3D, PLATFORM, _)

_PLATFORM Object3D * _Object3D(Object3D * that, VObjPoolImp * poolImp) {
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_OBJ3D_LINK);

	/////////////////////////////////////
#define Objec3d_Creator(T, P)\
	that->setTexture = Object3D_setTexture ##P;\
	that->setTextureD = Object3D_setTextureD ##P;\
	that->setBackfaceCulling = Object3D_setBackfaceCulling ##P;\
	that->setUV = Object3D_setUV ##P;\
	that->setNormalType = Object3D_setNormalType ##P; \
	that->setVertexType = Object3D_setVertexType ##P; \
	that->getTextureActual = Object3D_getTextureActual ##P;\
	that->getTexture = Object3D_getTexture ##P; \
	that->getTextureColor = Object3D_getTextureColor ##P; \
	that->setCenter = Object3D_setCenter ##P; \
	that->setLineColor = Object3D_setLineColor ##P; \
	that->setColor = Object3D_setColor ##P; \
	that->setColorB = Object3D_setColorB ##P; \
	that->addIndice = Object3D_addIndice ##P; \
	that->addIndiceN = Object3D_addIndiceN ##P; \
	that->setIndice = Object3D_setIndice ##P; \
	that->setIndiceI = Object3D_setIndiceI ##P; \
	that->addVertN = Object3D_addVertN ##P; \
	that->addVert = Object3D_addVert ##P; \
	that->addVertA = Object3D_addVertA ##P; \
	that->addVertI = Object3D_addVertI ##P; \
	that->renderAABB = Object3D_renderAABB ##P; \
	that->refreshAABBW = Object3D_refreshAABBW ##P; \
	that->refreshAABBC = Object3D_refreshAABBC ##P; \
	that->shaderVertex = Object3D_shaderVertex ##P; \
	that->render_normalize = Object3D_render_normalize ##P; \
	that->_move = Object3D_move ##P; \
	that->_scale = Object3D_scale ##P; \
	that->_rotate = Object3D_rotate ##P; \
	\
	that->_M.M = &that->M; \
	that->_M.M_1 = &that->M_1;\
	that->_M.mode = 1;\
	that->_M.Init(); \
	\
	that->texture = NULL; \
	that->anti = 1; \
	that->texture_type = 0; \
	\
	that->normal_type = 0; \
	that->vertex_type = 0; \
	that->backfaceculling = 0; \
	\
	that->cam = NULL;\
	\
	_VObjMan(&that->verts, 0, poolImp);\
	_VObjMan(&that->verts_r, 1, poolImp);\
	_VObjMan(&that->verts_f, 2, poolImp);\
	_VObjMan(&that->indice, 3, poolImp);\
	\
	that->transparent = 0;\
	that->reflection = 0;\
	that->v0 = NULL;\
	that->v1 = NULL;\
	\
	that->render_aabb = 0;\
	\
	that->center.init();\
	that->center_r.init();\
	that->center_w.init();\
	\
	that->u = 0;\
	that->v = 0;\
	\
	that->octTree = NULL; \
	\
	that->material = NULL; \
	\
	that->rightBottomFront_O.set(-EP_MAX, -EP_MAX, -EP_MAX, 1);\
	that->leftTopBack_O.set(EP_MAX, EP_MAX, EP_MAX, 1);
	/////////////////////////////////////
	Objec3d_Creator(Object3D, _)
	return that;
}

/////////////////////////////////////
typedef struct Obj3D Obj3D;
struct Obj3D {
	__SUPER(MultiLinkElement, Obj3D, NULL);
	Obj3D * _prev[MAX_OBJ3D_LINK];
	Obj3D * _next[MAX_OBJ3D_LINK];

	OBject3D_Define(Obj3D)
};
Function_Object3D(Obj3D, PLATFORM, __)

#define T Obj3D
#include "Object3D_funcs.h"
#undef T


_PLATFORM void Object3D_render_normalizeEx(Obj3D* that, int mode) {
	if (NULL == that->cam) {
		that->debugger = 2;
		return;
	}
	that->debugger = 3;
	that->CM.setI()* that->M* that->cam->M;

	if (that->render_aabb > 0) {
		that->render_aabb = -that->render_aabb;

		that->aabb[0].set(that->leftTopBack_O.x, that->leftTopBack_O.y, that->rightBottomFront_O.z);
		that->aabb[1].set(that->leftTopBack_O.x, that->rightBottomFront_O.y, that->rightBottomFront_O.z);
		that->aabb[3].set(that->rightBottomFront_O.x, that->leftTopBack_O.y, that->rightBottomFront_O.z);
		that->aabb[5].set(that->leftTopBack_O.x, that->rightBottomFront_O.y, that->leftTopBack_O.z);
		that->aabb[6].set(that->rightBottomFront_O.x, that->rightBottomFront_O.y, that->leftTopBack_O.z);
		that->aabb[7].set(that->rightBottomFront_O.x, that->leftTopBack_O.y, that->leftTopBack_O.z);
	}
	if (that->render_aabb) {
		if (mode == 1) {
			that->refreshAABBW(that);
		}
		else if (mode == 2) {
			that->refreshAABBC(that);
		}
		else if (mode == 3) {
			that->refreshAABBW(that);
			that->refreshAABBC(that);
		}
	}
}

_PLATFORM void Object3D_shaderVertexEx(Obj3D* that) {
	if (NULL == that->cam) {
		that->debugger = 2;
		return;
	}
	VObj* v = that->verts.link;
	if (v) {
		int i = 0;
		if (that->verts_r.linkcount > 0) {
			that->verts_r.linkcount = -that->verts_r.linkcount;
		}
		if (that->verts_f.linkcount > 0) {
			that->verts_f.linkcount = -that->verts_f.linkcount;
		}
		if (that->render_aabb) {
			for (i = 0; i < 8; i++) {
				v->v_r.set(that->aabb_r[i]);
				v->v_r.w = EP_MAX;
				if (!that->cam->normalize(that->cam, v->v_r)) {
					break;
				}
			}
			if (i == 8) {
				//that->verts_r.clearLink(&that->verts_r);
				//that->verts_f.clearLink(&that->verts_f);
				that->debugger = 4;
				return;
			}
		}
		that->v0 = NULL;
		that->v1 = NULL;
		int traverseCount = 0;
		do {
			traverseCount++;
			if (that->reflection > 0) {
				EFTYPE nx, ny, nz;
				EFTYPE x0, y0, z0;
				EFTYPE d;

				v->v_w.set(v->v) * that->CM;
				x0 = v->v_w.x;
				y0 = v->v_w.y;
				z0 = v->v_w.z;

				v->n_w.set(v->n) * that->CM;
				v->n_w.normalize();

				nx = v->n_w.x;
				ny = v->n_w.y;
				nz = v->n_w.z;
				d = -(v->n_w ^ v->v_w);

				v->R.mx.set(1 - 2 * nx * nx, -2 * ny * nx, -2 * nz * nx, -2 * d * nx);
				v->R.my.set(-2 * nx * ny, 1 - 2 * ny * ny, -2 * nz * ny, -2 * d * ny);
				v->R.mz.set(-2 * nx * nz, -2 * ny * nz, 1 - 2 * nz * nz, -2 * d * nz);
				v->R.mw.set(0, 0, 0, 1);

				v->n_w.set(v->n).negative() * that->CM;
				v->n_w.normalize();

				nx = v->n_w.x;
				ny = v->n_w.y;
				nz = v->n_w.z;
				d = -(v->n_w ^ v->v_w);
				v->R_r.mx.set(1 - 2 * nx * nx, -2 * ny * nx, -2 * nz * nx, -2 * d * nx);
				v->R_r.my.set(-2 * nx * ny, 1 - 2 * ny * ny, -2 * nz * ny, -2 * d * ny);
				v->R_r.mz.set(-2 * nx * nz, -2 * ny * nz, 1 - 2 * nz * nz, -2 * d * nz);
				v->R_r.mw.set(0, 0, 0, 1);
			}

			v->v_c.set(v->v) * that->CM;
			v->zz = v->v_c.z;
			v->v_r.set(v->v_c);
			that->debugger = v->v_r.z;
			v->cut = !that->cam->normalize_cut(that->cam, *v, *that->v0, *that->v1);
			v->cut = !that->cam->normalize(that->cam, v->v_r);
			if (!v->cut) {
				//printf("Cut: %p\n", v);
			} else {
				that->debugger = 6;
				v->n_w.set(v->n) ^ that->M;
				v->n_w.normalize();
				v->n_r.set(v->n) ^ that->CM;
				v->n_r.normalize();
				v->v_s.set(v->x * that->cam->scale_w + that->cam->offset_w, v->y * that->cam->scale_h + that->cam->offset_h, v->z);

				if (that->v0 && that->v1) {
					v->backface = that->backfaceculling > 0 ? (that->center_r.set(v->v_c).negative() ^ v->n_r) : 1;

					v->n_1_z.set(that->v0->x - v->x, that->v0->y - v->y, that->v0->z - v->z);
					that->center_r.set(that->v1->x - v->x, that->v1->y - v->y, that->v1->z - v->z);
					v->n_1_z * that->center_r;

					v->xs = EP_GT(Vert3D::get_minx(that->v0->v_s, that->v1->v_s, v->v_s), 0);
					v->xe = EP_GT(Vert3D::get_maxx(that->v0->v_s, that->v1->v_s, v->v_s), v->xs);
					v->ys = EP_GT(Vert3D::get_miny(that->v0->v_s, that->v1->v_s, v->v_s), 0);
					v->ye = EP_GT(Vert3D::get_maxy(that->v0->v_s, that->v1->v_s, v->v_s), v->ys);

					if (that->vertex_type == 1) {
						that->v0 = NULL;
						that->v1 = NULL;
					}
					else if (that->vertex_type == 2) {
						if ((traverseCount + 1) % 2 == 0) {
							that->v0 = v;
						}
						else {
							that->v0 = that->v1;
							that->v1 = v;
						}
					}
					else {
						that->v0 = that->v1;
						that->v1 = v;
					}
				}
				else if (NULL == that->v0) {
					that->v0 = v;
				}
				else if (NULL == that->v1) {
					that->v1 = v;
				}

				if (that->reflection) {
					if (that->verts_f.linkcount < 0) {
						that->verts_f.linkcount = -that->verts_f.linkcount;
						that->verts_f.clearLink(&that->verts_f);
					}
					if (that->verts_r.linkcount < 0) {
						that->verts_r.linkcount = -that->verts_r.linkcount;
						that->verts_r.clearLink(&that->verts_r);
					}
					that->verts_f.insertLink(&that->verts_f, v, NULL, NULL);
					that->verts_r.insertLink(&that->verts_r, v, NULL, NULL);
				}
				else {
					if (that->verts_r.linkcount < 0) {
						that->verts_r.linkcount = -that->verts_r.linkcount;
						that->verts_r.clearLink(&that->verts_r);
					}
					that->verts_r.insertLink(&that->verts_r, v, NULL, NULL);
				}
			}

			v = that->verts.next(&that->verts, v);
		} while (v && v != that->verts.link);

		if (that->verts_r.linkcount > 0) {
			if (!EP_ISZERO(that->transparent)) {
				that->center_r.set(that->center) * that->CM;
				if (!that->cam->normalize(that->cam, that->center_r)) {
					that->center_r.x = that->center_r.x * that->cam->scale_w + that->cam->offset_w;
					that->center_r.y = that->center_r.y * that->cam->scale_h + that->cam->offset_h;
				}
			}
		}
	}
}

inline Obj3D& Object3D_setTextureDEx(Obj3D* that, TextureManager& tman, INT uID, INT t) {
	Texture* ptexture = (Texture*)tman.textures.getLink(&tman.textures, uID);
	if (NULL == ptexture) {
		return *that;
	}
	that->t_w = ptexture->width;
	that->t_h = ptexture->height;
	that->texture = ptexture->texture;
	that->texture_type = t;
	that->setUV(that, 0, 0);
	return *that;
}
_PLATFORM Obj3D * _Obj3D(Obj3D * that, VObjPoolImp * poolImp) {
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_OBJ3D_LINK);

	Objec3d_Creator(Obj3D, _);

	that->setTextureD = Object3D_setTextureDEx;
		that->render_normalize = Object3D_render_normalizeEx;
		that->shaderVertex = Object3D_shaderVertexEx;
		return that;
}

_PLATFORM static void Object3D_GetInterpolationNormalVector(const VObj * v0, const VObj * v1, const VObj * v,
		INT __x, INT __y, INT line_r, INT line_l, INT _line_l1, INT _line_l, INT _line_l0, INT range,
		Vert3D& _n0, Vert3D& _n1, Vert3D& _n2, Vert3D& _n3) {
		const VObj * _v0, *_v1, *_v;

		if (EP_ISZERO_INT(line_r - line_l)) {
			_n0.set(v->n_r);
		}
		else {
			INT range = 5;
			//get left point position
			if (!EP_ISZERO_INT(_line_l1) && EP_RANGE_INT(_line_l1 - line_l, range)) {
				_v0 = v0;
				_v1 = v1;
			}
			else if (!EP_ISZERO_INT(_line_l) && EP_RANGE_INT(_line_l - line_l, range)) {
				_v0 = v1;
				_v1 = v;
			}
			else if (!EP_ISZERO_INT(_line_l0) && EP_RANGE_INT(_line_l0 - line_l, range)) {
				_v0 = v;
				_v1 = v0;
			}
			else {
				_v0 = v;
				_v1 = v0;
			}
			if (_v0->y0 > _v1->y0) {
				_v = _v0;
				_v0 = _v1;
				_v1 = _v;
			}
			if (EP_ISZERO(_v0->y0 - _v1->y0)) {
				_n2.set(v0->n_r);
			}
			else {
				_n0.set(_v0->n_r);
				_n1.set(_v1->n_r);
				//Ns = (1 / (y1 - y2))[N1 * (ys - y2) + N2 * (y1 - ys)]
				_n0 * ((__y - _v1->y0) / (_v0->y0 - _v1->y0));
				_n1 * ((_v0->y0 - __y) / (_v0->y0 - _v1->y0));
				_n2.set(_n0) + _n1;
			}
			//get right point position
			if (!EP_ISZERO_INT(_line_l1) && EP_RANGE_INT(_line_l1 - line_r, range)) {
				_v0 = v0;
				_v1 = v1;
			}
			else if (!EP_ISZERO_INT(_line_l) && EP_RANGE_INT(_line_l - line_r, range)) {
				_v0 = v1;
				_v1 = v;
			}
			else if (!EP_ISZERO_INT(_line_l0) && EP_RANGE_INT(_line_l0 - line_r, range)) {
				_v0 = v;
				_v1 = v0;
			}
			else {
				_v0 = v;
				_v1 = v0;
			}
			if (_v0->y0 > _v1->y0) {
				_v = _v0;
				_v0 = _v1;
				_v1 = _v;
			}
			if (EP_ISZERO(_v0->y0 - _v1->y0)) {
				_n3.set(v0->n_r);
			}
			else {
				_n0.set(_v0->n_r);
				_n1.set(_v1->n_r);
				//Na or Nb = (1 / (y1 - y2))[N1 * (ys - y2) + N2 * (y1 - ys)]
				_n0 * ((__y - _v1->y0) / (_v0->y0 - _v1->y0));
				_n1 * ((_v0->y0 - __y) / (_v0->y0 - _v1->y0));
				_n3.set(_n0) + _n1;
			}
			//get Ns
			_n0.set(_n2);
			_n1.set(_n3);
			//Ns = (1 / (xb - xa))[Na * (xb - xs) + Nb * (xs - xa)]
			_n0 * ((line_r - __x) / ((EFTYPE)(line_r - line_l)));
			_n1 * ((__x - line_l) / ((EFTYPE)(line_r - line_l)));
			_n0 + _n1;
			_n0.normalize();
		}
}
//Function_Object3D(Obj3D, _inline, ___)
//Obj3D * _Obj3DEx(Obj3D * that, VObjPoolImp * poolImp) {
//	that->prev = that->_prev;
//	that->next = that->_next;
//	_MultiLinkElementEx(&that->super, MAX_OBJ_LINK);
//
//	Objec3d_Creator(Obj3D, ___)
//
//	return that;
//}
typedef struct ObjPool ObjPool;
struct ObjPool {
	__SUPER(ElementPool, ObjPool, Obj3D);
};
_PLATFORM Obj3D * ObjPool_at(ObjPool * that, int index) {
	return &that->pool[index];
}
//Obj3D * ObjPool_atEx(ObjPool * that, int index) {
//	return &that->pool[index];
//}
_PLATFORM void _ObjPool(ObjPool * that, Obj3D * pool, UMAP * map, int size){
	_ElementPool(&that->super, (MultiLinkElement*)pool, map, size);

	that->at = ObjPool_at;
}

//void _ObjPoolEx(ObjPool * that, Obj3D * pool, UMAP * map, int size){
//	_ElementPoolEx(&that->super, (MultiLinkElement*)pool, map, size);
//
//	that->at = ObjPool_atEx;
//}

#define MAX_OBJ	100
#define MAP_OBJ	GET_MAP_SIZE(MAX_OBJ)
typedef struct ObjPoolImp ObjPoolImp;
struct ObjPoolImp {
	Obj3D pool[MAX_OBJ];
	UMAP map[MAP_OBJ];

	ObjPool objPool;
};
_PLATFORM ObjPoolImp* _ObjPoolImp(ObjPoolImp * that, VObjPoolImp * vpoolImp) {

	for (int i = 0; i < MAX_OBJ; i++) {
		_Obj3D(&that->pool[i], vpoolImp);
	}
	_ObjPool(&that->objPool, that->pool, that->map, MAX_OBJ);

	return that;
}
typedef struct ObjMan ObjMan;
struct ObjMan{
	__SUPER(MultiLinkBase, ObjMan, Obj3D);

	ObjPool * objPool;
	VObjPoolImp * vobjPoolImp;

	////////////////////////////
	void (*clearLink)(ObjMan * that);
	////////////////////////////
};
_PLATFORM void ObjMan_clearlink(ObjMan * that){
	if (that->link) {
		Obj3D * temp = that->link;
		do {
			if (that->removeLink(that, temp) == NULL) {
				break;
			}
			if (!temp->free(temp)) {
				temp->indice.clearLink(&temp->indice);
				temp->verts.clearLink(&temp->verts);
				temp->verts_r.clearLink(&temp->verts_r);
				temp->verts_f.clearLink(&temp->verts_f);
				that->objPool->back(that->objPool, temp);
			}

			temp = that->link;
		} while (temp);
	}
}
//void ObjMan_clearlinkEx(ObjMan * that){
//	Obj3D * o = that->link;
//	Obj3D * n = NULL;
//	if (o) {
//		do {
//			n = that->next(that, o);
//
//			if (that->removeLink(that, o)) {
//				if (!o->free(o)) {
//					that->objPool->back(that->objPool, o);
//				}
//			}
//
//			o = n;
//		} while (o && o != that->link);
//	}
//}
_PLATFORM ObjMan * _ObjMan(ObjMan * that, int index, ObjPoolImp * poolImp, VObjPoolImp * vpoolImp){
	_MultiLinkBase(&that->super, index);

	that->objPool = &poolImp->objPool;
	that->vobjPoolImp = vpoolImp;

	///////////////////////////////////////
	that->clearLink = ObjMan_clearlink;
	///////////////////////////////////////

	return that;
}
//ObjMan * _ObjManEx(ObjMan * that, int index, ObjPoolImp * poolImp, VObjPoolImp * vpoolImp){
//	_MultiLinkBaseEx(&that->super, index);
//
//	that->objPool = &poolImp->objPool;
//
//	for (int i = 0; i < MAX_OBJ; i++) {
//		_Obj3DEx(&poolImp->pool[i], vpoolImp);
//	}
//	_ObjPoolEx(that->objPool, poolImp->pool, poolImp->map, MAX_OBJ);
//
//	///////////////////////////////////////
//	that->clearLink = ObjMan_clearlinkEx;
//	///////////////////////////////////////
//
//	return that;
//}
/////////////////////////////////////

#define MAX_GROUP3D_LINK 1
typedef struct Group3D Group3D;
struct Group3D {
	__SUPER(MultiLinkElement, Group3D, NULL);
	Group3D * _prev[MAX_GROUP3D_LINK];
	Group3D * _next[MAX_GROUP3D_LINK];
	/////////////////////////////
	ObjMan objs;
	/////////////////////////////
};
_PLATFORM Group3D * _Group3D(Group3D * that, ObjPoolImp * objPoolImp, VObjPoolImp * vobjPoolImp) {
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_GROUP3D_LINK);

	/////////////////////////////////////
	_ObjMan(&that->objs, 1, objPoolImp, vobjPoolImp);
	/////////////////////////////////////

	return that;
}

typedef struct Group3DPool Group3DPool;
struct Group3DPool {
	__SUPER(ElementPool, Group3DPool, Group3D);
};
_PLATFORM Group3D * Group3DPool_at(Group3DPool * that, int index) {
	return &that->pool[index];
}
_PLATFORM void _Group3DPool(Group3DPool * that, Group3D * pool, UMAP * map, int size) {
	_ElementPool(&that->super, (MultiLinkElement *)pool, map, size);

	that->at = Group3DPool_at;
}

#define MAX_GRP 100
#define MAP_GRP GET_MAP_SIZE(MAX_GRP)
typedef struct Group3DPoolImp Group3DPoolImp;
struct Group3DPoolImp {
	Group3D pool[MAX_GRP];
	UMAP map[MAP_GRP];

	Group3DPool group3DPool;
};
_PLATFORM Group3DPoolImp * _Group3DPoolImp(Group3DPoolImp *that, ObjPoolImp * objPoolImp, VObjPoolImp * vobjPoolImp) {
	for (int i = 0; i < MAX_GRP; i++) {
		_Group3D(&that->pool[i], objPoolImp, vobjPoolImp);
	}
	_Group3DPool(&that->group3DPool, that->pool, that->map, MAX_GRP);

	return that;
}
typedef struct Group3DMan Group3DMan;
struct Group3DMan {
	__SUPER(MultiLinkBase, Group3DMan, Group3D);

	Group3DPool * group3DPool;

	////////////////////////////
	void(*clearLink)(Group3DMan * that);
	////////////////////////////
};
_PLATFORM void Group3DMan_clearlink(Group3DMan * that) {
	if (that->link) {
		Group3D * temp = that->link;
		do {
			if (that->removeLink(that, temp) == NULL) {
				break;
			}
			if (!temp->free(temp)) {
				that->group3DPool->back(that->group3DPool, temp);
			}

			temp = that->link;
		} while (temp);
	}
}
_PLATFORM Group3DMan * _Group3DMan(Group3DMan * that, int index, Group3DPoolImp * group3DPoolImp) {
	_MultiLinkBase(&that->super, index);

	that->group3DPool = &group3DPoolImp->group3DPool;

	///////////////////////////////////////
	that->clearLink = Group3DMan_clearlink;
	///////////////////////////////////////

	return that;
}


_PLATFORM INT Collision(Vert3D& vo, Vert3D& vd, Obj3D * _obj, EFTYPE max) {
	if (NULL == _obj) {
		return 0;
	}

	//012 023 326 367 034 437 045 051 475 576 152 256
	static INT indice[12][3] = {
		{ 0, 1, 2 },
		{ 0, 2, 3 },
		{ 3, 2, 6 },
		{ 3, 6, 7 },
		{ 0, 3, 4 },
		{ 4, 3, 7 },
		{ 0, 4, 5 },
		{ 0, 5, 1 },
		{ 4, 7, 5 },
		{ 5, 7, 6 },
		{ 1, 5, 2 },
		{ 2, 5, 6 }
	};
	INT intersect = 0;
	Vert3D v0, v1, v2;
	Vert3D n0, n1, n, p;
	for (int i = 0; i < 12; i++) {
		v0.set(_obj->aabb_r[indice[i][0]]);
		v1.set(_obj->aabb_r[indice[i][1]]);
		v2.set(_obj->aabb_r[indice[i][2]]);

		n0.set(v1) - v0;
		n1.set(v2) - v0;
		n.set(n0) * n1;
		//back face culling
		EFTYPE cross = vd  & n;
		if (cross * _obj->anti < 0) {
			continue;
		}
		EFTYPE trans = Vert3D::GetLineIntersectPointWithTriangle(v0, v1, v2, vo, vd, max, p);
		if (EP_GTZERO(trans)) {
			intersect = 1;
			break;
		}
	}
	if (intersect) {
		return 1;
	}
	return 0;
}

#endif