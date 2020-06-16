//OctTree.h
//Oct Treee 3D scene management

#ifndef _OCTTREE_H_
#define _OCTTREE_H_

#include "../math3d/Object3D.h"

class Rect3D {
public:
	_PLATFORM Rect3D(EFTYPE x, EFTYPE y, EFTYPE z, EFTYPE w, EFTYPE h, EFTYPE d) :
	x(x), y(y), z(z), width(w), height(h), depth(d){

	}
	_PLATFORM Rect3D() :
	x(0), y(0), z(0), width(0), height(0), depth(0){
	}

	EFTYPE x;
	EFTYPE y;
	EFTYPE z;
	EFTYPE width;
	EFTYPE height;
	EFTYPE depth;

	_PLATFORM void set(EFTYPE x, EFTYPE y, EFTYPE z, EFTYPE w, EFTYPE h, EFTYPE d) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->width = w;
		this->height = h;
		this->depth = d;
	}

	_PLATFORM void set(const Rect3D& r) {
		this->x = r.x;
		this->y = r.y;
		this->z = r.z;
		this->width = r.width;
		this->height = r.height;
		this->depth = r.depth;
	}
};

enum OctTree_Link {
	RightTopFront = 0,
	LeftTopFront = 1,
	LeftBottomFront = 2,
	RightBottomFront = 3,
	RightTopBack = 4,
	LeftTopBack = 5,
	LeftBottomBack = 6,
	RightBottomBack = 7,
	OctTreeLinkMax = 8
};

#define MAX_QUARDANTS	8
#define MAX_OBJECTS		5
#define MAX_LEVELS		(MAX_OBJ3D_LINK - MAX_OBJ3D_END) / (MAX_QUARDANTS + 1)
#define GET_LINK_INDEX(level, position)	(level * MAX_QUARDANTS + position + MAX_OBJ3D_END)

#define MAX_OCT_LINK 1
typedef struct OctTree OctTree;
struct OctTree {
	__SUPER(MultiLinkElement, OctTree, NULL);
	OctTree * _prev[MAX_OCT_LINK];
	OctTree * _next[MAX_OCT_LINK];

	/////////////////////////////
	INT level;
	INT position;
	Rect3D bounds;
	INT maxObjects;
	INT maxLevels;
	ObjPoolImp * objPoolImp;
	VObjPoolImp * vobjPoolImp;
	ObjMan objects;
	OctTree * parent;
	INT hasChild;
	OctTree * children[OctTreeLinkMax];
	Rect3D rect;
	/////////////////////////////////////
};
_PLATFORM OctTree * _OctTreeA(OctTree * that, ObjPoolImp * objPoolImp, VObjPoolImp * vobjPoolImp) {
	if (that == NULL) {
		return that;
	}
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_OBJ3D_LINK);

	////////////////////////////////
	that->level = 0;
	that->position = -1;
	that->maxObjects = MAX_OBJECTS;
	that->maxLevels = MAX_LEVELS;
	that->parent = NULL;
	that->hasChild = 0;
	that->objPoolImp = objPoolImp;
	that->vobjPoolImp = vobjPoolImp;
	_ObjMan(&that->objects, GET_LINK_INDEX(that->level, that->position), that->objPoolImp, that->vobjPoolImp);
	////////////////////////////////
	return that;
}
_PLATFORM OctTree * _OctTree(OctTree * that, INT level,
	EFTYPE x, EFTYPE y, EFTYPE z, EFTYPE w, EFTYPE h, EFTYPE d,
	OctTree * parent, INT position, ObjPoolImp * objPoolImp, VObjPoolImp * vobjPoolImp) {
	if (that == NULL) {
		return that;
	}
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_OBJ3D_LINK);

	////////////////////////////////
	that->level = level;
	that->position = position;
	that->bounds.set(x, y, z, w, h, d);
	that->maxObjects = MAX_OBJECTS;
	that->maxLevels = MAX_LEVELS;
	that->parent = parent;
	that->hasChild = 0;
	that->objPoolImp = objPoolImp;
	that->vobjPoolImp = vobjPoolImp;
	_ObjMan(&that->objects, GET_LINK_INDEX(that->level, that->position), that->objPoolImp, that->vobjPoolImp);
	for (int i = 0; i < OctTreeLinkMax; i++) {
		that->children[i] = NULL;
	}
	////////////////////////////////
	return that;
}

typedef struct OctPool OctPool;
struct OctPool {
	__SUPER(ElementPool, OctPool, OctTree);
};
_PLATFORM OctTree * OctPool_at(OctPool * that, int index) {
	return &that->pool[index];
}
_PLATFORM void _OctPool(OctPool * that, OctTree * pool, UMAP * map, int size) {
	_ElementPool(&that->super, (MultiLinkElement*)pool, map, size);

	that->at = OctPool_at;
}
#define MAX_OCT	1000
#define MAP_OCT	GET_MAP_SIZE(MAX_OCT)
typedef struct OctPoolImp OctPoolImp;
struct OctPoolImp {
	OctTree pool[MAX_OCT];
	UMAP map[MAX_OCT];

	OctPool octPool;
};
_PLATFORM OctPoolImp* _OctPoolImp(OctPoolImp * that, ObjPoolImp * objPoolImp, VObjPoolImp * vobjPoolImp) {

	for (int i = 0; i < MAX_OCT; i++) {
		_OctTreeA(&that->pool[i], objPoolImp, vobjPoolImp);
	}
	_OctPool(&that->octPool, that->pool, that->map, MAX_OCT);

	return that;
}
typedef struct OctMan OctMan;
struct OctMan {
	__SUPER(MultiLinkBase, OctMan, OctTree);

	OctPool * octPool;
	OctPoolImp * octPoolImp;
	ObjPoolImp * objPoolImp;
	VObjPoolImp * vobjPoolImp;

	////////////////////////////
	void (*clearLink)(OctMan * that);
	void (*set)(OctMan * that, OctTree* oct, INT level,
		EFTYPE x, EFTYPE y, EFTYPE z, EFTYPE w, EFTYPE h, EFTYPE d,
		OctTree * parent, INT position);
	void (*Split)(OctMan* that, OctTree *oct);
	INT (*GetIndex)(OctMan *, OctTree * oct, Rect3D& rect);
	void (*Insert)(OctMan * that, OctTree * oct, Obj3D * obj);
	void (*CollisionA)(OctMan * that, OctTree * oct, Obj3D * obj, ObjMan * link);
	void(*change)(OctMan * that, OctTree * oct, Obj3D * obj);
	////////////////////////////
};
_PLATFORM void OctMan_clearlink(OctMan * that) {
	if (that->link) {
		OctTree * temp = that->link;
		do {
			if (that->removeLink(that, temp) == NULL) {
				break;
			}
			if (!temp->free(temp)) {
				temp->objects.clearLink(&temp->objects);
				that->octPool->back(that->octPool, temp);
			}

			temp = that->link;
		} while (temp);
	}
}

_PLATFORM void OctMan_set(OctMan * that, OctTree* oct, INT level,
	EFTYPE x, EFTYPE y, EFTYPE z, EFTYPE w, EFTYPE h, EFTYPE d,
	OctTree * parent, INT position) {
	oct->level = level;
	oct->objects.linkindex = GET_LINK_INDEX(level, position);
	oct->position = position;
	oct->bounds.set(x, y, z, w, h, d);
	oct->parent = parent;
}

// 5 4
// 6 7
// 1 0
// 2 3
_PLATFORM void OctMan_Split(OctMan* that, OctTree *oct) {
	EFTYPE subWidth = oct->bounds.width / 2;
	EFTYPE subHeight = oct->bounds.height / 2;
	EFTYPE subDepth = oct->bounds.depth / 2;
	EFTYPE x = oct->bounds.x;
	EFTYPE y = oct->bounds.y;
	EFTYPE z = oct->bounds.z;

	oct->children[OctTree_Link::RightTopFront] = 
		_OctTree(that->octPool->get(that->octPool), oct->level + 1, x, y + subHeight, z + subDepth, subWidth, subHeight, subDepth, oct, OctTree_Link::RightTopFront, that->objPoolImp, that->vobjPoolImp);

	oct->children[OctTree_Link::LeftTopFront] = 
		_OctTree(that->octPool->get(that->octPool), oct->level + 1, x + subWidth, y + subHeight, z + subDepth, subWidth, subHeight, subDepth, oct, OctTree_Link::LeftTopFront, that->objPoolImp, that->vobjPoolImp);

	oct->children[OctTree_Link::LeftBottomFront] = 
		_OctTree(that->octPool->get(that->octPool), oct->level + 1, x + subWidth, y, z + subDepth, subWidth, subHeight, subDepth, oct, OctTree_Link::LeftBottomFront, that->objPoolImp, that->vobjPoolImp);;

	oct->children[OctTree_Link::RightBottomFront] = 
		_OctTree(that->octPool->get(that->octPool), oct->level + 1, x, y, z + subDepth, subWidth, subHeight, subDepth, oct, OctTree_Link::RightBottomFront, that->objPoolImp, that->vobjPoolImp);

	oct->children[OctTree_Link::RightTopBack] = 
		_OctTree(that->octPool->get(that->octPool), oct->level + 1, x, y + subHeight, z, subWidth, subHeight, subDepth, oct, OctTree_Link::RightTopBack, that->objPoolImp, that->vobjPoolImp);

	oct->children[OctTree_Link::LeftTopBack] = 
		_OctTree(that->octPool->get(that->octPool), oct->level + 1, x + subWidth, y + subHeight, z, subWidth, subHeight, subDepth, oct, OctTree_Link::LeftTopBack, that->objPoolImp, that->vobjPoolImp);

	oct->children[OctTree_Link::LeftBottomBack] = 
		_OctTree(that->octPool->get(that->octPool), oct->level + 1, x + subWidth, y, z, subWidth, subHeight, subDepth, oct, OctTree_Link::LeftBottomBack, that->objPoolImp, that->vobjPoolImp);

	oct->children[OctTree_Link::RightBottomBack] = 
		_OctTree(that->octPool->get(that->octPool), oct->level + 1, x, y, z, subWidth, subHeight, subDepth, oct, OctTree_Link::RightBottomBack, that->objPoolImp, that->vobjPoolImp);

	oct->hasChild = true;
}

_PLATFORM INT OctMan_GetIndex(OctMan *, OctTree * oct, Rect3D& rect) {
	INT index = -1;
	EFTYPE xMidPoint = oct->bounds.x + oct->bounds.width / 2;
	EFTYPE yMidPoint = oct->bounds.y + oct->bounds.height / 2;
	EFTYPE zMidPoint = oct->bounds.z + oct->bounds.depth / 2;

	EFTYPE bottomQuad = (EP_GEZERO(rect.y - oct->bounds.y) && EP_LEZERO(rect.y + rect.height - yMidPoint));
	EFTYPE topQuad = (EP_GEZERO(rect.y - yMidPoint) && EP_LEZERO(rect.y + rect.height - (oct->bounds.y + oct->bounds.height)));
	EFTYPE backQuad = (EP_GEZERO(rect.z - oct->bounds.z) && EP_LEZERO(rect.z + rect.depth - zMidPoint));
	EFTYPE frontQuad = (EP_GEZERO(rect.z - zMidPoint) && EP_LEZERO(rect.z + rect.depth - (oct->bounds.z + oct->bounds.depth)));

	if (EP_GEZERO(rect.x - xMidPoint) && EP_LEZERO(rect.x + rect.width - (oct->bounds.x + oct->bounds.width))) {//leftQuad
		if (topQuad) {
			if (frontQuad) {
				index = OctTree_Link::LeftTopFront;//leftTopFront
			}
			else if (backQuad) {
				index = OctTree_Link::LeftTopBack;//leftTopBack
			}
		}
		else if (bottomQuad) {
			if (frontQuad) {
				index = OctTree_Link::LeftBottomFront;//leftBottomFront
			}
			else if (backQuad) {
				index = OctTree_Link::LeftBottomBack;//leftBottomBack
			}
		}
	}
	else if (EP_GEZERO(rect.x - oct->bounds.x) && EP_LEZERO(rect.x + rect.width - xMidPoint)) {//rightQuad
		if (topQuad) {
			if (frontQuad) {
				index = OctTree_Link::RightTopFront;//rightTopFront
			}
			else if (backQuad) {
				index = OctTree_Link::RightTopBack;//rightTopBack
			}
		}
		else if (bottomQuad) {
			if (frontQuad) {
				index = OctTree_Link::RightBottomFront;//rightBottmFront
			}
			else if (backQuad) {
				index = OctTree_Link::RightBottomBack;//rightBottomBack
			}
		}
	}
	return index;
}

_PLATFORM void OctMan_Insert(OctMan * that, OctTree * oct, Obj3D * obj) {
	if (oct == NULL) {
		return;
	}
	if (NULL == obj) {
		return;
	}
	if (oct->hasChild) {
		oct->rect.set(obj->leftTopBack.x, obj->leftTopBack.y, obj->leftTopBack.z,
			obj->rightBottomFront.x - obj->leftTopBack.x, obj->rightBottomFront.y - obj->leftTopBack.y, obj->rightBottomFront.z - obj->leftTopBack.z);
		INT index = that->GetIndex(that, oct, oct->rect);// oct->GetIndex(that, oct, oct->rect);
		if (index != -1) {
			//oct->children[index]->Insert(obj);
			that->Insert(that, oct->children[index], obj);
			obj->octTree = oct;
		}
	}
	oct->objects.insertLink(&oct->objects, obj, NULL, NULL);
	if (oct->objects.linkcount > oct->maxObjects && oct->level < oct->maxLevels) {
		//oct->Split();
		that->Split(that, oct);
		Obj3D * _obj = oct->objects.link, *_next;
		if (_obj) {
			do {
				_next = oct->objects.next(&oct->objects, _obj);

				oct->rect.set(_obj->leftTopBack.x, _obj->leftTopBack.y, _obj->leftTopBack.z,
					_obj->rightBottomFront.x - _obj->leftTopBack.x, _obj->rightBottomFront.y - _obj->leftTopBack.y, _obj->rightBottomFront.z - _obj->leftTopBack.z);
				INT index = that->GetIndex(that, oct, oct->rect);// oct->GetIndex(oct->rect);
				if (index != -1) {
					oct->objects.removeLink(&oct->objects, _obj);
					//oct->children[index]->Insert(_obj);
					that->Insert(that, oct->children[index], _obj);
				}

				_obj = _next;
			} while (_obj && _obj != oct->objects.link);
		}
	}
}

_PLATFORM void OctMan_CollisionA(OctMan * that, OctTree * oct, Obj3D * obj, ObjMan * link) {
	if (NULL == obj) {
		return;
	}
	if (NULL == link) {
		return;
	}
	if (oct->hasChild) {
		oct->rect.set(obj->leftTopBack.x, obj->leftTopBack.y, obj->leftTopBack.z,
			obj->rightBottomFront.x - obj->leftTopBack.x, obj->rightBottomFront.y - obj->leftTopBack.y, obj->rightBottomFront.z - obj->leftTopBack.z);
		INT index = that->GetIndex(that, oct, oct->rect);// oct->GetIndex(oct->rect);
		if (index != -1) {
			//oct->children[index]->Collision(obj, link);
			that->CollisionA(that, oct->children[index], obj, link);
		}
	}
	if (NULL == oct->objects.link) {
		return;
	}
	Obj3D * _obj = oct->objects.link;
	if (_obj) {
		do {

			if (_obj != obj) {
				link->insertLink(link, _obj, NULL, NULL);
			}

			_obj = oct->objects.next(&oct->objects, _obj);
		} while (_obj && _obj != oct->objects.link);
	}
}


_PLATFORM void OctMan_change(OctMan* that, OctTree * oct, Obj3D * obj) {
	if (NULL == obj) {
		return;
	}
	if (NULL == obj->octTree) {
		//oct->Insert(obj);
		that->Insert(that, oct, obj);
	}
	else
	{
		((OctTree*)obj->octTree)->objects.removeLink(&((OctTree*)obj->octTree)->objects, obj);
		if (((OctTree*)obj->octTree)->parent) {
			//((OctTree*)obj->octTree)->parent->Insert(obj);
			that->Insert(that, ((OctTree*)obj->octTree)->parent, obj);
		}
		else {
			//this->Insert(obj);
			that->Insert(that, oct, obj);
		}
	}
}
_PLATFORM OctMan * _OctMan(OctMan * that, int index, OctPoolImp * poolImp, ObjPoolImp * objPoolImp, VObjPoolImp * vobjPoolImp) {
	_MultiLinkBase(&that->super, index);

	that->octPool = &poolImp->octPool;

	///////////////////////////////////////
	that->clearLink = OctMan_clearlink;
	that->set = OctMan_set;
	that->Split = OctMan_Split;
	that->GetIndex = OctMan_GetIndex;
	that->Insert = OctMan_Insert;
	that->CollisionA = OctMan_CollisionA;
	that->change = OctMan_change;
	///////////////////////////////////////

	that->objPoolImp = objPoolImp;
	that->vobjPoolImp = vobjPoolImp;
	that->insertLink(that, _OctTreeA(that->octPool->get(that->octPool), objPoolImp, vobjPoolImp), NULL, NULL);

	return that;
}

#endif