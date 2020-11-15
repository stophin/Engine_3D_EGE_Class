#ifndef CAMERA_H
#define CAMERA_H

#include "Ray.h"

class camera {
public:
	camera() {
		auto aspect_ratio = 16.0 / 9.0;
		auto viewport_height = 2.0;
		auto viewport_width = aspect_ratio * viewport_height;
		auto focal_length = 1.0;

		origin.set(0, 0, 0);
		horizontal.set(viewport_width, 0.0, 0.0);
		vertical.set(0.0, -viewport_height, 0.0);
		Vert3D v0, v1, v2;
		v0.set(horizontal) / 2;
		v1.set(vertical) / 2;
		v2.set(0, 0, focal_length);
		lower_left_corner.set(origin) - v0 - v1 - v2;
	}
	void set(
		Vert3D& lookfrom,
		Vert3D& lookat,
		Vert3D&   vup,
		double vfov, // vertical field-of-view in degrees
		double aspect_ratio,
		double aperture,
		double focus_dist
	) {
		auto theta = degrees_to_radians(vfov);
		auto h = tan(theta / 2);
		auto viewport_height = 2.0 * h;
		auto viewport_width = aspect_ratio * viewport_height;

		(w.set(lookfrom) - lookat).normalize();
		(u.set(vup) * w).normalize();
		(v.set(w) * u);

		origin.set(lookfrom);
		horizontal.set(u) * (focus_dist * viewport_width);
		vertical.set(v) * (focus_dist * (-viewport_height));

		Vert3D v0, v1, v2;
		v0.set(horizontal) / 2;
		v1.set(vertical) / 2;
		v2.set(w) * focus_dist;
		lower_left_corner.set(origin) - v0 - v1 - v2;

		lens_radius = aperture / 2;
	}

	void set(
		Vert3D& lookfrom,
		Vert3D& lookat,
		Vert3D&   vup,
		double vfov, // vertical field-of-view in degrees
		double aspect_ratio
	) {
		auto theta = degrees_to_radians(vfov);
		auto h = tan(theta / 2);
		auto viewport_height = 2.0 * h;
		auto viewport_width = aspect_ratio * viewport_height;

		(w.set(lookfrom) - lookat).normalize();
		(u.set(vup) * w).normalize();
		(v.set(w) * u);

		origin.set(lookfrom);
		horizontal.set(u) * (viewport_width);
		vertical.set(v) * (-viewport_height);

		Vert3D v0, v1;
		v0.set(horizontal) / 2;
		v1.set(vertical) / 2;
		lower_left_corner.set(origin) - v0 - v1 - w;
	}

	void set(
		double vfov, // vertical field-of-view in degrees
		double aspect_ratio
	) {
		auto theta = degrees_to_radians(vfov);
		auto h = tan(theta / 2);
		auto viewport_height = 2.0 * h;
		auto viewport_width = aspect_ratio * viewport_height;

		auto focal_length = 1.0;

		origin.set(0, 0, 0);
		horizontal.set(viewport_width, 0.0, 0.0);
		vertical.set(0.0, -viewport_height, 0.0);

		u.set(horizontal) / 2;
		v.set(vertical) / 2;
		w.set(0, 0, focal_length);
		lower_left_corner.set(origin) - u - v;
	}


	void get_ray(Ray& ray, double s, double t) const {
		Vert3D rd, offset, v0, v1;
		random_in_unit_disk(rd);
		rd * lens_radius;
		offset.set(u) * rd.x;
		v0.set(v) * rd.y;
		offset + v0;

		v0.set(horizontal) * s;
		v1.set(vertical) * t;
		v0 + v1 + lower_left_corner - origin - offset;
		v1.set(origin) + offset;

		ray.set(v1, v0);
	}

	void get_ray_2(Ray& ray, double s, double t) const {
		Vert3D v0, v1, v2;
		v0.set(horizontal) * s;
		v1.set(vertical) * t;
		v2.set(lower_left_corner) + v0 + v1 - origin;
		ray.set(origin, v2);
	}

	void get_ray_1(Ray& ray, double u, double v) const {
		Vert3D v0, v1, v2;
		v0.set(horizontal) * u;
		v1.set(vertical) * v;
		v2.set(lower_left_corner) + v0 + v1 - origin;
		ray.set(origin, v2);
	}

private:
	Vert3D origin;
	Vert3D lower_left_corner;
	Vert3D horizontal;
	Vert3D vertical;
	Vert3D u, v, w;
	double lens_radius;
};
#endif