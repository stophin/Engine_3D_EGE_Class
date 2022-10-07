_PLATFORM void Camera3D_initialize_(T* that, int type) {
	that->type = type;
	if (type == 1) {
		that->proj.mx.set(2 * that->znear / that->width, 0, -(that->rt.x + that->lb.x) / that->width, 0);
		that->proj.my.set(0, 2 * that->znear / that->height, -(that->rt.y + that->lb.y) / that->height, 0);
		that->proj.mz.set(0, 0, that->zfar / (that->zfar - that->znear), -that->zfar * that->znear / (that->zfar - that->znear));
		that->proj.mw.set(0, 0, 1, 0);

		that->proj_1.mx.set(1 / (2 * that->znear / that->width), 0, -(that->rt.x + that->lb.x) / that->width, 0);
		that->proj_1.my.set(0, 1 / (2 * that->znear / that->height), -(that->rt.y + that->lb.y) / that->height, 0);
		that->proj_1.mz.set(0, 0, 1 / (that->zfar / (that->zfar - that->znear)), (-that->zfar * that->znear / (that->zfar - that->znear)) / (that->zfar / (that->zfar - that->znear)));
		that->proj_1.mw.set(0, 0, 0, 1);
	}
	else if (type == 2) {
		that->proj.mx.set(2 * that->znear / that->width, 0, -(that->rt.x + that->lb.x) / that->width, 0);
		that->proj.my.set(0, 2 * that->znear / that->height, -(that->rt.y + that->lb.y) / that->height, 0);
		that->proj.mz.set(0, 0, (that->zfar + that->znear) / (that->zfar - that->znear), -2 * that->zfar * that->znear / (that->zfar - that->znear));
		that->proj.mw.set(0, 0, 1, 0);

		that->proj_1.mx.set(1 / (2 * that->znear / that->width), 0, -(that->rt.x + that->lb.x) / that->width, 0);
		that->proj_1.my.set(0, 1 / (2 * that->znear / that->height), -(that->rt.y + that->lb.y) / that->height, 0);
		that->proj_1.mz.set(0, 0, 1 / ((that->zfar + that->znear) / (that->zfar - that->znear)), -(-2 * that->zfar * that->znear / (that->zfar - that->znear)) / ((that->zfar + that->znear) / (that->zfar - that->znear)));
		that->proj_1.mw.set(0, 0, 0, 1);
	}
	else {
		that->proj.mx.set(2 / that->width, 0, 0, -(that->rt.x + that->lb.x) / that->width);
		that->proj.my.set(0, 2 / that->height, 0, -(that->rt.y + that->lb.y) / that->height);
		that->proj.mz.set(0, 0, 1 / (that->zfar - that->znear), -that->znear / (that->zfar - that->znear));
		that->proj.mw.set(0, 0, 0, 1);
	}
}
_PLATFORM void Camera3D_project_(T* that, Vert3D& v) {
	EFTYPE z = 1.0 / v.z;
	v* that->proj;
	v* z;
}
_PLATFORM int Camera3D_normalize_cut_(T* that, VObj& v, const VObj& v0, const VObj& v1) {
	if (that->lookat.z > 0) {
		if (EP_GTZERO(v.z - that->zfar)) {
			v.z = that->zfar;
		}
		else if (EP_GTZERO(v.z - that->znear)) {
			v.z = that->znear;
		}
	}
	else if (that->lookat.z < 0) {
		if (EP_GTZERO(v.z + that->znear)) {
			v.z = -that->znear;
		}
		else if (EP_LTZERO(v.z + that->zfar)) {
			v.z = -that->zfar;
		}
	}
	EFTYPE w = that->tanwidth * v.z, h = that->tanheight * v.z;
	if (w < 0) w = -w;
	if (h < 0) h = -h;
	if (v.x > w) {
		v.x = w;
	}
	else if (v.x < -w) {
		v.x = -w;
	}
	else if (v.y > h) {
		v.y = h;
	}
	else if (v.y < -h) {
		v.y = -h;
	}
	if (EP_ISZERO(v.z)) {
		return -1;
	}
	else if (!EP_ISZERO(v.w - EP_MAX)) {
		that->project(that, v.v_r);
	}
	return 0;
}
_PLATFORM int Camera3D_normalize_(T* that, Vert3D& v) {
	if (that->lookat.z > 0 && (EP_GTZERO(v.z - that->zfar) || EP_GTZERO(v.z - that->znear))) {
		return -2;
	}
	else if (that->lookat.z < 0 && (EP_GTZERO(v.z + that->znear) || EP_GTZERO(v.z + that->zfar))) {
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
_PLATFORM int Camera3D_anti_normalize_(T* that, Vert3D& v, EFTYPE z) {
	v* z;
	v.w = 1;
	v* that->proj_1;
	v.w = 1;
	return 0;
}
_PLATFORM void Camera3D_setRange_(T* that, EFTYPE o_w, EFTYPE o_h, EFTYPE s_w, EFTYPE s_h) {
	that->g.set(o_w, o_h, s_w, s_h);
}
_PLATFORM T& Camera3D_move_(T* that, EFTYPE dx, EFTYPE dy, EFTYPE dz) {
	that->_M.move(dx, dy, dz);

	return *that;
}
_PLATFORM T& Camera3D_rotate_(T* that, EFTYPE ax, EFTYPE ay, EFTYPE az) {
	that->_M.rotate(ax, ay, az);

	return *that;
}
_PLATFORM void Camera3D_set_(T* that, EFTYPE width, EFTYPE height, EFTYPE znear, EFTYPE zfar, EFTYPE angle_width, EFTYPE angle_height) {
	that->width = width;
	that->height = height;
	that->znear = znear;
	that->zfar = zfar;
	that->lb.set(-width / 2.0, -height / 2.0, 0, 1);
	that->rt.set(width / 2.0, height / 2.0, 0, 1);
	that->tanwidth = tan(angle_width * EP_PI / 180.0);
	that->tanheight = tan(angle_height * EP_PI / 180.0);

	that->initialize(that, 2);
}