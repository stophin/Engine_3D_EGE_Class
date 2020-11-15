#ifndef BVH_H
#define BVH_H

#include "Ray.h"
#include "hittable.h"
#include "hittable_list.h"


class bvh_node : public hittable {
public:
	bvh_node() {}

	virtual bool hit(
		Ray& r, double t_min, double t_max, hit_record& rec) override {
		if (!box.hit(r, t_min, t_max))
			return false;

		bool hit_left = left->hit(r, t_min, t_max, rec);
		bool hit_right = right->hit(r, t_min, hit_left ? rec.t : t_max, rec);

		return hit_left || hit_right;
	}


	bool bounding_box(double t0, double t1, aabb& output_box) {
		output_box.set(box);
		return true;
	}

public:
	hittable* left;
	hittable* right;
	aabb box;
};


/////////////////////////////////////////////////////
#define MAX_BVH_NODE_LINK	4
typedef struct BvhNode BvhNode;
struct BvhNode {
	__SUPER(MultiLinkElement, BvhNode, NULL);
	BvhNode * _prev[MAX_BVH_NODE_LINK];
	BvhNode * _next[MAX_BVH_NODE_LINK];

	/////////////////////////////////////
	bvh_node obj;
	/////////////////////////////////////
};
_PLATFORM BvhNode * _BvhNode(BvhNode * that, bvh_node * o) {
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_BVH_NODE_LINK);

	/////////////////////////////////////
	/////////////////////////////////////

	return that;
}

/////////////////////////////////////
typedef struct BvhNodePool BvhNodePool;
struct BvhNodePool {
	__SUPER(ElementPool, BvhNodePool, BvhNode);
};
_PLATFORM BvhNode * BvhNodePool_at(BvhNodePool * that, int index) {
	return &that->pool[index];
}
_PLATFORM void _BvhNodePool(BvhNodePool * that, BvhNode * pool, UMAP * map, int size) {
	_ElementPool(&that->super, (MultiLinkElement*)pool, map, size);

	that->at = BvhNodePool_at;
}

#define MAX_BVH_NODE	1000
#define MAP_BVH_NODE	GET_MAP_SIZE(MAX_BVH_NODE)
typedef struct BvhNodePoolImp BvhNodePoolImp;
struct BvhNodePoolImp {
	BvhNode pool[MAX_BVH_NODE];
	UMAP map[MAP_BVH_NODE];

	BvhNodePool bvh_nodePool;
};
_PLATFORM BvhNodePoolImp * _BvhNodePoolImp(BvhNodePoolImp *that) {

	for (int i = 0; i < MAX_BVH_NODE; i++) {
		_BvhNode(&that->pool[i], NULL);
	}
	_BvhNodePool(&that->bvh_nodePool, that->pool, that->map, MAX_BVH_NODE);

	return that;
}

typedef struct BvhNodeMan BvhNodeMan;
struct BvhNodeMan {
	__SUPER(MultiLinkBase, BvhNodeMan, BvhNode);

	BvhNodePool * bvh_nodePool;

	////////////////////////////
	void(*clearLink)(BvhNodeMan * that);
	////////////////////////////
};
_PLATFORM void BvhNodeMan_clearlink(BvhNodeMan * that) {
	if (that->link) {
		BvhNode * temp = that->link;
		do {
			if (that->removeLink(that, temp) == NULL) {
				break;
			}
			if (!temp->free(temp)) {
				that->bvh_nodePool->back(that->bvh_nodePool, temp);
			}

			temp = that->link;
		} while (temp);
	}
}
_PLATFORM BvhNodeMan * _BvhNodeMan(BvhNodeMan * that, int index, BvhNodePoolImp * poolImp) {
	_MultiLinkBase(&that->super, index);

	that->bvh_nodePool = &poolImp->bvh_nodePool;

	///////////////////////////////////////
	that->clearLink = BvhNodeMan_clearlink;
	///////////////////////////////////////

	return that;
}
/////////////////////////////////////



inline bool box_compare(Hittable* a, Hittable* b, int axis) {
	aabb box_a;
	aabb box_b;

	if (!a->obj->bounding_box(0, 0, box_a) || !b->obj->bounding_box(0, 0, box_b))
		printf("No bounding box in bvh_node constructor.\n");

	return box_a._min.e[axis] < box_b._min.e[axis];
}


inline bool box_x_compare(Hittable* a, Hittable* b) {
	return box_compare(a, b, 0);
}

inline bool box_y_compare(Hittable* a, Hittable* b) {
	return box_compare(a, b, 1);
}

inline bool box_z_compare(Hittable* a, Hittable* b) {
	return box_compare(a, b, 2);
}


inline void set_bvh(bvh_node * bvh, BvhNodeMan * bvhMan,  int dep,
	HittableMan& src_objects,
	size_t start, size_t end, double time0, double time1) {

	int axis = random_int(0, 2);
	auto comparator = (axis == 0) ? box_x_compare
		: (axis == 1) ? box_y_compare
		: box_z_compare;

	size_t object_span = end - start;

	if (object_span == 1) {
		Hittable* h0 = src_objects.get(&src_objects, start);
		bvh->left = bvh->right = h0->obj;
	}
	else if (object_span == 2) {
		Hittable* h0, *h1;
		h0 = src_objects.get(&src_objects, start);
		h1 = src_objects.get(&src_objects, start + 1);
		if (comparator(h0, h1)) {
			bvh->left = h0->obj;
			bvh->right = h1->obj;
		}
		else {
			bvh->left = h1->obj;
			bvh->right = h0->obj;
		}
	}
	else {
		// Create a modifiable array of the source scene objects
		if (dep >= MAX_HITTABLE_MAX) {
			return;
		}
		//bvh itself will speedup the rendering, so it will OK without sorting
		//but it will work best if the the two children have smaller bounding boxes than their parents
		//so here we will randomly choose an axis and sort the primitives to achieve that point
		HittableMan &objects = src_objects;
		/*
		HittableMan objects;
		_HittableMan(&objects, 3 + dep, src_objects.hittablePoolImp);
		Hittable * object = src_objects.link;
		if (object) {
			do {

				objects.insertLink(&objects, object, NULL, NULL);

				object = src_objects.next(&src_objects, object);
			} while (object && object != src_objects.link);
		}
		//std::sort(objects.begin() + start, objects.begin() + end, comparator);
		objects.sort(&objects, start, end, comparator);*/

		size_t mid = start + object_span / 2.0;
		BvhNode * bvhNode;
		bvhNode = bvhMan->bvh_nodePool->get(bvhMan->bvh_nodePool);
		_BvhNode(bvhNode, NULL);
		set_bvh(&bvhNode->obj, bvhMan, ++dep, objects, start, mid, time0, time1);
		bvh->left = &bvhNode->obj;
		bvhNode = bvhMan->bvh_nodePool->get(bvhMan->bvh_nodePool);
		_BvhNode(bvhNode, NULL);
		set_bvh(&bvhNode->obj, bvhMan, ++dep, objects, mid, end, time0, time1);
		bvh->right = &bvhNode->obj;
	}

	aabb box_left, box_right;

	if (!bvh->left->bounding_box(time0, time1, box_left)
		|| !bvh->right->bounding_box(time0, time1, box_right)
		)
		printf("No bounding box in bvh_node constructor.\n");

	surrounding_box(box_left, box_right, bvh->box);
}

inline void set_bvh(bvh_node * bvh, BvhNodeMan * bvhMan,
	hittable_list& list, double time0, double time1) {
	set_bvh(bvh, bvhMan, 0, list.objects, 0, list.objects.linkcount, time0, time1);
}
#endif