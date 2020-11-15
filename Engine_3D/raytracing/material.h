#ifndef MATERIAL_H
#define MATERIAL_H

#include "Ray.h"
#include "hittable.h"

class material {
public:
	virtual bool scatter(Ray& r_in, const hit_record& rec, Vert3D& attenuation, Ray& scattered) = 0;
};

class lambertian : public material {
public:
	lambertian() {}

	void set(Vert3D& c) {
		albedo.set(c);
	}
	void set(EFTYPE r, EFTYPE g, EFTYPE b) {
		albedo.set(r, g, b);
	}

	virtual bool scatter(
		Ray& r_in, const hit_record& rec, Vert3D& attenuation, Ray& scattered
	) override {
		Vert3D scatter_direction;
		random_unit_vector(scatter_direction);
		scatter_direction + rec.normal;
		scattered.set(rec.p, scatter_direction);
		scattered.type = 0;
		scattered.obj = r_in.obj;
		if (rec.use_color) {
			attenuation.set(rec.color);
		}
		else {
			attenuation.set(albedo);
		}
		return true;
	}

public:
	Vert3D albedo;
};
class metal : public material {
public:
	metal() {}

	void set(Vert3D& c) {
		albedo.set(c);
	}
	void set(Vert3D& c, EFTYPE f) {
		albedo.set(c);
		fuzz = f;
	}
	void set(EFTYPE r, EFTYPE g, EFTYPE b, EFTYPE f) {
		albedo.set(r, g, b);
		fuzz = f;
	}

	virtual bool scatter(
		Ray& r_in, const hit_record& rec, Vert3D& attenuation, Ray& scattered
	) override {
		Vert3D reflected;
		attenuation.set(r_in.direction).normalize();
		reflect(attenuation, rec.normal, reflected);
		random_in_unit_sphere(attenuation);
		attenuation * fuzz;
		reflected + attenuation;
		scattered.set(rec.p, reflected);
		scattered.type = 1;
		scattered.obj = r_in.obj;
		attenuation.set(albedo);
		return (scattered.direction ^ rec.normal) > 0;
	}

public:
	Vert3D albedo;
	EFTYPE fuzz;
};
class dielectric : public material {
public:
	dielectric(){}

	void set(double index_of_refraction) {
		ir = index_of_refraction;
	}

	virtual bool scatter(
		Ray& r_in, const hit_record& rec, Vert3D& attenuation, Ray& scattered
	) override {
		double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

		Vert3D unit_direction;
		attenuation.set(r_in.direction).normalize();
		unit_direction.set(attenuation);
		attenuation.negative();
		double cos_theta = fmin((attenuation ^ rec.normal), 1.0);
		double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

		bool cannot_refract = refraction_ratio * sin_theta > 1.0;

		if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
			reflect(unit_direction, rec.normal, attenuation);
		else
			refract(unit_direction, rec.normal, refraction_ratio, attenuation);

		scattered.set(rec.p, attenuation);
		scattered.type = 2;
		scattered.obj = r_in.obj;
		attenuation.set(1.0, 1.0, 1.0);
		return true;
	}

public:
	double ir; // Index of Refraction

private:
	static double reflectance(double cosine, double ref_idx) {
		// Use Schlick's approximation for reflectance.
		auto r0 = (1 - ref_idx) / (1 + ref_idx);
		r0 = r0 * r0;
		return r0 + (1 - r0)*pow((1 - cosine), 5);
	}
};

/////////////////////////////////////////////////////
#define MAX_LAMBERTIAN_LINK	4
typedef struct Lambertian Lambertian;
struct Lambertian {
	__SUPER(MultiLinkElement, Lambertian, NULL);
	Lambertian * _prev[MAX_LAMBERTIAN_LINK];
	Lambertian * _next[MAX_LAMBERTIAN_LINK];

	/////////////////////////////////////
	lambertian obj;
	/////////////////////////////////////
};
_PLATFORM Lambertian * _Lambertian(Lambertian * that, lambertian * o) {
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_LAMBERTIAN_LINK);

	/////////////////////////////////////
	/////////////////////////////////////

	return that;
}

/////////////////////////////////////
typedef struct LambertianPool LambertianPool;
struct LambertianPool {
	__SUPER(ElementPool, LambertianPool, Lambertian);
};
_PLATFORM Lambertian * LambertianPool_at(LambertianPool * that, int index) {
	return &that->pool[index];
}
_PLATFORM void _LambertianPool(LambertianPool * that, Lambertian * pool, UMAP * map, int size) {
	_ElementPool(&that->super, (MultiLinkElement*)pool, map, size);

	that->at = LambertianPool_at;
}

#define MAX_LAMBERTIAN	1000
#define MAP_LAMBERTIAN	GET_MAP_SIZE(MAX_LAMBERTIAN)
typedef struct LambertianPoolImp LambertianPoolImp;
struct LambertianPoolImp {
	Lambertian pool[MAX_LAMBERTIAN];
	UMAP map[MAP_LAMBERTIAN];

	LambertianPool lambertianPool;
};
_PLATFORM LambertianPoolImp * _LambertianPoolImp(LambertianPoolImp *that) {

	for (int i = 0; i < MAX_LAMBERTIAN; i++) {
		_Lambertian(&that->pool[i], NULL);
	}
	_LambertianPool(&that->lambertianPool, that->pool, that->map, MAX_LAMBERTIAN);

	return that;
}

typedef struct LambertianMan LambertianMan;
struct LambertianMan {
	__SUPER(MultiLinkBase, LambertianMan, Lambertian);

	LambertianPool * lambertianPool;

	////////////////////////////
	void(*clearLink)(LambertianMan * that);
	////////////////////////////
};
_PLATFORM void LambertianMan_clearlink(LambertianMan * that) {
	if (that->link) {
		Lambertian * temp = that->link;
		do {
			if (that->removeLink(that, temp) == NULL) {
				break;
			}
			if (!temp->free(temp)) {
				that->lambertianPool->back(that->lambertianPool, temp);
			}

			temp = that->link;
		} while (temp);
	}
}
_PLATFORM LambertianMan * _LambertianMan(LambertianMan * that, int index, LambertianPoolImp * poolImp) {
	_MultiLinkBase(&that->super, index);

	that->lambertianPool = &poolImp->lambertianPool;

	///////////////////////////////////////
	that->clearLink = LambertianMan_clearlink;
	///////////////////////////////////////

	return that;
}
/////////////////////////////////////

/////////////////////////////////////////////////////
#define MAX_METAL_LINK	4
typedef struct Metal Metal;
struct Metal {
	__SUPER(MultiLinkElement, Metal, NULL);
	Metal * _prev[MAX_METAL_LINK];
	Metal * _next[MAX_METAL_LINK];

	/////////////////////////////////////
	metal obj;
	/////////////////////////////////////
};
_PLATFORM Metal * _Metal(Metal * that, metal * o) {
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_METAL_LINK);

	/////////////////////////////////////
	/////////////////////////////////////

	return that;
}

/////////////////////////////////////
typedef struct MetalPool MetalPool;
struct MetalPool {
	__SUPER(ElementPool, MetalPool, Metal);
};
_PLATFORM Metal * MetalPool_at(MetalPool * that, int index) {
	return &that->pool[index];
}
_PLATFORM void _MetalPool(MetalPool * that, Metal * pool, UMAP * map, int size) {
	_ElementPool(&that->super, (MultiLinkElement*)pool, map, size);

	that->at = MetalPool_at;
}

#define MAX_METAL	1000
#define MAP_METAL	GET_MAP_SIZE(MAX_METAL)
typedef struct MetalPoolImp MetalPoolImp;
struct MetalPoolImp {
	Metal pool[MAX_METAL];
	UMAP map[MAP_METAL];

	MetalPool metalPool;
};
_PLATFORM MetalPoolImp * _MetalPoolImp(MetalPoolImp *that) {

	for (int i = 0; i < MAX_METAL; i++) {
		_Metal(&that->pool[i], NULL);
	}
	_MetalPool(&that->metalPool, that->pool, that->map, MAX_METAL);

	return that;
}

typedef struct MetalMan MetalMan;
struct MetalMan {
	__SUPER(MultiLinkBase, MetalMan, Metal);

	MetalPool * metalPool;

	////////////////////////////
	void(*clearLink)(MetalMan * that);
	////////////////////////////
};
_PLATFORM void MetalMan_clearlink(MetalMan * that) {
	if (that->link) {
		Metal * temp = that->link;
		do {
			if (that->removeLink(that, temp) == NULL) {
				break;
			}
			if (!temp->free(temp)) {
				that->metalPool->back(that->metalPool, temp);
			}

			temp = that->link;
		} while (temp);
	}
}
_PLATFORM MetalMan * _MetalMan(MetalMan * that, int index, MetalPoolImp * poolImp) {
	_MultiLinkBase(&that->super, index);

	that->metalPool = &poolImp->metalPool;

	///////////////////////////////////////
	that->clearLink = MetalMan_clearlink;
	///////////////////////////////////////

	return that;
}
/////////////////////////////////////

/////////////////////////////////////////////////////
#define MAX_DIELECTRIC_LINK	4
typedef struct Dielectric Dielectric;
struct Dielectric {
	__SUPER(MultiLinkElement, Dielectric, NULL);
	Dielectric * _prev[MAX_DIELECTRIC_LINK];
	Dielectric * _next[MAX_DIELECTRIC_LINK];

	/////////////////////////////////////
	dielectric obj;
	/////////////////////////////////////
};
_PLATFORM Dielectric * _Dielectric(Dielectric * that, dielectric * o) {
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_DIELECTRIC_LINK);

	/////////////////////////////////////
	/////////////////////////////////////

	return that;
}

/////////////////////////////////////
typedef struct DielectricPool DielectricPool;
struct DielectricPool {
	__SUPER(ElementPool, DielectricPool, Dielectric);
};
_PLATFORM Dielectric * DielectricPool_at(DielectricPool * that, int index) {
	return &that->pool[index];
}
_PLATFORM void _DielectricPool(DielectricPool * that, Dielectric * pool, UMAP * map, int size) {
	_ElementPool(&that->super, (MultiLinkElement*)pool, map, size);

	that->at = DielectricPool_at;
}

#define MAX_DIELECTRIC	1000
#define MAP_DIELECTRIC	GET_MAP_SIZE(MAX_DIELECTRIC)
typedef struct DielectricPoolImp DielectricPoolImp;
struct DielectricPoolImp {
	Dielectric pool[MAX_DIELECTRIC];
	UMAP map[MAP_DIELECTRIC];

	DielectricPool dielectricPool;
};
_PLATFORM DielectricPoolImp * _DielectricPoolImp(DielectricPoolImp *that) {

	for (int i = 0; i < MAX_DIELECTRIC; i++) {
		_Dielectric(&that->pool[i], NULL);
	}
	_DielectricPool(&that->dielectricPool, that->pool, that->map, MAX_DIELECTRIC);

	return that;
}

typedef struct DielectricMan DielectricMan;
struct DielectricMan {
	__SUPER(MultiLinkBase, DielectricMan, Dielectric);

	DielectricPool * dielectricPool;

	////////////////////////////
	void(*clearLink)(DielectricMan * that);
	////////////////////////////
};
_PLATFORM void DielectricMan_clearlink(DielectricMan * that) {
	if (that->link) {
		Dielectric * temp = that->link;
		do {
			if (that->removeLink(that, temp) == NULL) {
				break;
			}
			if (!temp->free(temp)) {
				that->dielectricPool->back(that->dielectricPool, temp);
			}

			temp = that->link;
		} while (temp);
	}
}
_PLATFORM DielectricMan * _DielectricMan(DielectricMan * that, int index, DielectricPoolImp * poolImp) {
	_MultiLinkBase(&that->super, index);

	that->dielectricPool = &poolImp->dielectricPool;

	///////////////////////////////////////
	that->clearLink = DielectricMan_clearlink;
	///////////////////////////////////////

	return that;
}
/////////////////////////////////////

#endif