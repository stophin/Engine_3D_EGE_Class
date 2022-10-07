_PLATFORM void Light3D_render_normalize_(T* that) {
	if (that->cam == NULL) {
		return;
	}
	that->pos_r.set(that->pos);
	that->pos_r* that->M* that->cam->M;
}
_PLATFORM T& Light3D_move_(T* that, EFTYPE dx, EFTYPE dy, EFTYPE dz) {
	that->_M.move(dx, dy, dz);

	that->render_normalize(that);

	return *that;
}
_PLATFORM T& Light3D_rotate_(T* that, EFTYPE dx, EFTYPE dy, EFTYPE dz) {
	that->_M.rotate(dx, dy, dz);

	that->render_normalize(that);

	return *that;
}
_PLATFORM EFTYPE Light3D_getFactor_(T* that, const Vert3D& n, const Vert3D& n0) {
	EFTYPE f = 0, _f = 0, __f = 0;
	that->n1.set(that->pos_r);
	that->n1 - n0;
	_f = (n & that->n1);
	if (that->mode > 0) {
		that->r.set(n);
		that->r* (2 * (that->r ^ that->n1)) - that->n1;
		__f = (that->r & n);
		__f *= __f;
	}
	f += _f + __f;
	return f;
}