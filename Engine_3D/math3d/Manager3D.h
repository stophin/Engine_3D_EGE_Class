// Manager3D.h
// Manager for Camera and Object
//

#ifndef _MANAGER3D_H_
#define _MANAGER3D_H_

#include "Object3D.h"

#include "../scene/OctTree.h"

typedef class Manager3D Manager3D;
class Manager3D {
public:
	_PLATFORM Manager3D(){
	}
	_PLATFORM ~Manager3D() {
	}
	_PLATFORM void Init() {
		_CamMan(&cams, 0, camPoolImp);
		_ObjMan(&objs, 0, objPoolImp, vobjPoolImp);
		_ObjMan(&tras, 0, objPoolImp, vobjPoolImp);
		_ObjMan(&refl, 0, objPoolImp, vobjPoolImp);
		_LgtMan(&lgts, 0, lgtPoolImp);
		_ObjMan(&octs, 2, objPoolImp, vobjPoolImp);
		_OctMan(&octTree, 0, octPoolImp, objPoolImp, vobjPoolImp);
		_Group3DMan(&grps, 0, group3DPoolImp);
		group = NULL;
		changed = 0;
		this->ax = 0;
		this->ay = 0;
		this->az = 0;
	}	
	//void InitEx() {
	//	_CamManEx(&cams, 0, camPoolImp);
	//	_ObjManEx(&objs, 0, objPoolImp, vobjPoolImp);
	//	_ObjManEx(&tras, 0, objPoolImp, vobjPoolImp);
	//	_ObjManEx(&refl, 0, objPoolImp, vobjPoolImp);
	//	_LgtManEx(&lgts, 0, lgtPoolImp);
	//}
	CamPoolImp* camPoolImp;
	ObjPoolImp* objPoolImp;
	VObjPoolImp* vobjPoolImp;
	LgtPoolImp* lgtPoolImp;
	OctPoolImp * octPoolImp;
	Group3DPoolImp * group3DPoolImp;
	Group3D * group;
	int initialized;

	CamMan cams;
	ObjMan objs;
	ObjMan tras;
	ObjMan refl;
	LgtMan lgts;
	Group3DMan grps;
	Group3D group3DTemp;
	Cam3D camTemp;
	Obj3D objTemp;
	Lgt3D lgtTemp;
	//oct-tree
	ObjMan octs;

	INT changed;

	Rect3D rect;
	OctMan octTree;
	_PLATFORM void createOctTree() {
		//get scnene aabb
		Obj3D* _obj = this->objs.link;
		INT render_state = 0;
		this->rect.set(EP_MAX, EP_MAX, EP_MAX, 0, 0, 0);
		if (_obj) {
			do {
				Vert3D &v0 = _obj->leftTopBack;
				Vert3D &v1 = _obj->rightBottomFront;
				if (this->rect.x > v0.x) this->rect.x = v0.x;
				if (this->rect.y > v0.y) this->rect.y = v0.y;
				if (this->rect.z > v0.z) this->rect.z = v0.z;
				if (this->rect.width < v1.x) this->rect.width = v1.x;
				if (this->rect.height < v1.y) this->rect.height = v1.y;
				if (this->rect.depth < v1.z) this->rect.depth = v1.z;

				//first do objects till end
				//then do reflection and then transparent object
				if (render_state == 0) {
					_obj = this->objs.next(&this->objs, _obj);
					if (!(_obj && _obj != this->objs.link)) {
						_obj = this->refl.link;
						render_state = 1;
						if (!_obj) {
							//or render reflection points
							_obj = this->tras.link;
							render_state = 2;
						}
					}
				}
				else if (render_state == 1) {
					_obj = this->refl.next(&this->refl, _obj);
					if (!(_obj && _obj != this->refl.link)) {
						_obj = this->tras.link;
						render_state = 2;
					}
				}
				else {
					_obj = this->tras.next(&this->tras, _obj);
					if (!(_obj && _obj != this->tras.link)) {
						break;
					}
				}
			} while (_obj);
		}
		this->rect.width = this->rect.width - this->rect.x;
		this->rect.height = this->rect.height - this->rect.y;
		this->rect.depth = this->rect.depth - this->rect.z;
		//create oct-tree
		this->octTree.link->bounds.set(this->rect);
		_obj = this->objs.link;
		render_state = 0;
		if (_obj) {
			do {
				this->octTree.Insert(&this->octTree, this->octTree.link, _obj);

				//first do objects till end
				//then do reflection and then transparent object
				if (render_state == 0) {
					_obj = this->objs.next(&this->objs, _obj);
					if (!(_obj && _obj != this->objs.link)) {
						_obj = this->refl.link;
						render_state = 1;
						if (!_obj) {
							//or render reflection points
							_obj = this->tras.link;
							render_state = 2;
						}
					}
				}
				else if (render_state == 1) {
					_obj = this->refl.next(&this->refl, _obj);
					if (!(_obj && _obj != this->refl.link)) {
						_obj = this->tras.link;
						render_state = 2;
					}
				}
				else {
					_obj = this->tras.next(&this->tras, _obj);
					if (!(_obj && _obj != this->tras.link)) {
						break;
					}
				}
			} while (_obj);
		}
	}

	_PLATFORM void setCameraRange(EFTYPE o_w, EFTYPE o_h, EFTYPE s_w, EFTYPE s_h) {
		Cam3D * cam = this->cams.link;
		if (cam){
			do {
				cam->setRange(cam, o_w, o_h, s_w, s_h);

				cam = this->cams.next(&this->cams, cam);
			} while (cam && cam != this->cams.link);
		}

		this->refresh(0);
	}

	_PLATFORM Camera3D& addCamera(EFTYPE width, EFTYPE height, EFTYPE znear, EFTYPE zfar, EFTYPE angle_width, EFTYPE angle_height) {
		//Cam3D * cam = new Cam3D(width, height, znear, zfar, angle_width, angle_height);'
		Cam3D * cam = this->cams.camPool->get(this->cams.camPool);
		if (NULL == cam) {
			return *((Camera3D*)&this->camTemp);
		}
		_Cam3D(cam);
		cam->set(cam, width, height, znear, zfar, angle_width, angle_height);
		this->cams.insertLink(&this->cams, cam, NULL, NULL);

		return *((Camera3D*)cam);
	}
	_PLATFORM Camera3D& addShadowCamera() {
		//Cam3D * cam = new Cam3D(width, height, znear, zfar, angle_width, angle_height);'
		Cam3D * cam = this->cams.camPool->get(this->cams.camPool);
		if (NULL == cam) {
			return *((Camera3D*)&this->camTemp);
		}
		_Cam3D(cam);
		cam->type = 1;
		this->cams.insertLink(&this->cams, cam, NULL, NULL);

		return *((Camera3D*)cam);
	}
	_PLATFORM Camera3D& addReflectionCamera() {
		//Cam3D * cam = new Cam3D(width, height, znear, zfar, angle_width, angle_height);'
		Cam3D * cam = this->cams.camPool->get(this->cams.camPool);
		if (NULL == cam) {
			return *((Camera3D*)&this->camTemp);
		}
		_Cam3D(cam);
		cam->type = 2;
		this->cams.insertLink(&this->cams, cam, NULL, NULL);

		return *((Camera3D*)cam);
	}

	_PLATFORM Camera3D * getCamera(int type) {
		Cam3D * cam = this->cams.link;
		if (cam) {
			do {
				if (cam->type == type) {
					break;
				}

				cam = this->cams.next(&this->cams, cam);
			} while (cam && cam != this->cams.link);
		}
		if (cam->type != type) {
			return NULL;
		}
		return (Camera3D*)cam;
	}

	_PLATFORM Group3D& addGroup() {
		Group3D* gp = this->grps.group3DPool->get(this->grps.group3DPool);
		if (NULL == gp) {
			return this->group3DTemp;
		}
		_Group3D(gp, this->objPoolImp, this->vobjPoolImp);
		this->grps.insertLink(&this->grps, gp, NULL, NULL);
		return *gp;
	}
	_PLATFORM Manager3D& startGroup(INT uniqueID) {
		this->group = this->grps.getLink(&this->grps, uniqueID);

		return *this;
	}
	_PLATFORM Manager3D& endGroup() {
		this->group = NULL;
		return *this;
	}

	_PLATFORM Object3D& addObject() {
		//Obj3D * obj = new Obj3D();
		Obj3D * obj = this->objs.objPool->get(this->objs.objPool);
		if (NULL == obj) {
			return *((Object3D*)&this->objTemp);
		}
		_Obj3D(obj, this->objs.vobjPoolImp);

		obj->cam = (Camera3D*)this->cams.link;

		this->objs.insertLink(&this->objs, obj, NULL, NULL);
		if (this->group) {
			this->group->objs.insertLink(&this->group->objs, obj, NULL, NULL);
		}

		//TODO
		((Object3D *)obj)->render_aabb = 1;

		return *((Object3D *)obj);
	}

	_PLATFORM Object3D& addObjectI(INT anti) {
		Object3D& obj = this->addObject();
		obj.anti = anti;
		return obj;
	}

	_PLATFORM Object3D& addReflectionObject(EFTYPE t){
		//Obj3D * obj = new Obj3D();
		Obj3D * obj = this->objs.objPool->get(this->objs.objPool);
		if (NULL == obj) {
			return *((Object3D*)&this->objTemp);
		}
		_Obj3D(obj, this->objs.vobjPoolImp);

		obj->cam = (Camera3D*)this->cams.link;

		obj->reflection = t;

		this->refl.insertLink(&this->objs, obj, NULL, NULL);
		if (this->group) {
			this->group->objs.insertLink(&this->group->objs, obj, NULL, NULL);
		}
		((Object3D*)obj)->anti = 1;
		//TODO
		((Object3D*)obj)->render_aabb = 1;

		return *((Object3D *)obj);

	}


	_PLATFORM Object3D& addTransparentObject(EFTYPE t){
		//Obj3D * obj = new Obj3D();
		Obj3D * obj = this->objs.objPool->get(this->objs.objPool);
		if (NULL == obj) {
			return *((Object3D*)&this->objTemp);
		}
		_Obj3D(obj, this->objs.vobjPoolImp);

		obj->cam = (Camera3D*)this->cams.link;

		obj->transparent = t;

		this->tras.insertLink(&this->objs, obj, NULL, NULL);
		if (this->group) {
			this->group->objs.insertLink(&this->group->objs, obj, NULL, NULL);
		}
		((Object3D*)obj)->anti = 1;
		//TODO
		((Object3D*)obj)->render_aabb = 1;

		return *((Object3D *)obj);

	}

	_PLATFORM Light3D& addLight(EFTYPE x, EFTYPE y, EFTYPE z) {
		//Lgt3D * lgt = new Lgt3D();
		Lgt3D * lgt = this->lgts.lgtPool->get(this->lgts.lgtPool);
		if (NULL == lgt) {
			return *((Light3D*)lgt);
		}
		_Lgt3D(lgt);

		lgt->cam = (Camera3D*)this->cams.link;

		lgt->_move(lgt, x, y, z);

		this->lgts.insertLink(&this->lgts, lgt, NULL, NULL);

		return *((Light3D*)lgt);
	}

	_PLATFORM void moveCamera(EFTYPE x, EFTYPE y, EFTYPE z) {
		if (this->cams.link) {
			this->cams.link->_move(this->cams.link, x, y, z);

			this->refresh(0);
		}
	}

	EFTYPE ax;
	EFTYPE ay;
	EFTYPE az;
	_PLATFORM void rotateCamera(EFTYPE ax, EFTYPE ay, EFTYPE az) {
		if (this->cams.link) {
			this->ax += ax;
			this->ay += ay;
			this->az += az;
			this->cams.link->_rotate(this->cams.link, ax, ay, az);

			this->refresh(0);
		}
	}

	_PLATFORM void nextCamera() {

		do {
			if (this->cams.link) {
				Cam3D * cam = this->cams.removeLink(&this->cams, this->cams.link);
				this->cams.insertLink(&this->cams, cam, NULL, NULL);
			}
		} while (this->cams.link->type != 0);

		this->refresh(NULL);
	}

	_PLATFORM void refresh(Camera3D* cam) {
		Obj3D * obj = this->objs.link;
		if (obj) {
			do {
				if (cam) {
					obj->cam = cam;
				}
				else {
					obj->cam = (Camera3D*)this->cams.link;
				}
				obj->render_normalize(obj, 2);

				obj = this->objs.next(&this->objs, obj);
			} while (obj && obj != this->objs.link);
		}
		obj = this->tras.link;
		if (obj) {
			do {
				if (cam) {
					obj->cam = cam;
				}
				else {
					obj->cam = (Camera3D*)this->cams.link;
				}
				obj->render_normalize(obj, 2);

				obj = this->tras.next(&this->tras, obj);
			} while (obj && obj != this->tras.link);
		}
		obj = this->refl.link;
		if (obj) {
			do {
				if (cam) {
					obj->cam = cam;
				} 
				else {
					obj->cam = (Camera3D*)this->cams.link;
				}
				obj->render_normalize(obj, 2);

				obj = this->refl.next(&this->refl, obj);
			} while (obj && obj != this->refl.link);
		}
		Lgt3D * lgt = this->lgts.link;
		if (lgt) {
			do {
				if (cam) {
					lgt->cam = cam;
				} 
				else {
					lgt->cam = (Camera3D*)this->cams.link;
				}
				lgt->render_normalize(lgt);

				lgt = this->lgts.next(&this->lgts, lgt);
			} while (lgt && lgt != this->lgts.link);
		}

		changed++;
	}

	_PLATFORM void moveLight(EFTYPE x, EFTYPE y, EFTYPE z) {
		if (this->lgts.link) {
			this->lgts.link->_move(this->lgts.link, x, y, z);
		}
	}
	_PLATFORM void rotateLight(EFTYPE x, EFTYPE y, EFTYPE z) {
		if (this->lgts.link) {
			this->lgts.link->_rotate(this->lgts.link, x, y, z);
		}
	}

	_PLATFORM void nextLight() {
		if (this->lgts.link) {
			Lgt3D * lgt = this->lgts.removeLink(&this->lgts, this->lgts.link);
			this->lgts.insertLink(&this->lgts, lgt, NULL, NULL);
		}
	}
};

#endif