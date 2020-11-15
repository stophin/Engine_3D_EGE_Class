#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"
#include "aabb.h"

class hittable_list : public hittable {
public:
	hittable_list() {}
	void set(hittable* object) { add(object); }

	void clear() { objects.clearLink(&objects); }
	void add(hittable* object) { 
		Hittable * hittable = objects.hittablePool->get(objects.hittablePool);
		if (hittable) {
			hittable->obj = object;
			objects.insertLink(&objects, hittable, NULL, NULL);
		}
	}

	bool hit(Ray& r, double t_min, double t_max, hit_record& rec) override {
		bool hit_anything = false;
		auto closest_so_far = t_max;

		Hittable * object = objects.link;
		if (object) {
			do {
				if (object->obj->hit(r, t_min, closest_so_far, rec)) {
					hit_anything = true;
					closest_so_far = rec.t;
				}

				object = objects.next(&objects, object);
			} while (object && object != objects.link);
		}

		return hit_anything;
	}

	bool bounding_box(double t0, double t1, aabb& output_box) {
		if (objects.linkcount <= 0) return false;

		aabb temp_box;
		bool first_box = true;

		Hittable * object = objects.link;
		if (object) {
			do {
				if (!object->obj->bounding_box(t0, t1, temp_box)) return false;
				first_box ? output_box.set(temp_box) : surrounding_box(output_box, temp_box, output_box);
				first_box = false;

				object = objects.next(&objects, object);
			} while (object && object != objects.link);
		}

		return true;
	}

public:
	HittableMan objects;
};

#endif