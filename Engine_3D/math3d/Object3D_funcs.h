_PLATFORM T& Object3D_setBackfaceCulling_(T* that, INT bfc) {
	that->backfaceculling = bfc;
	return *that;
}
_PLATFORM T& Object3D_setUV_(T* that, EFTYPE u, EFTYPE v) {
	that->u = u;
	that->v = v;
	return *that;
}
_PLATFORM T& Object3D_setNormalType_(T* that, INT n) {
	that->normal_type = n;
	return *that;
}
_PLATFORM T& Object3D_setVertexType_(T* that, INT type) {
	that->vertex_type = type;
	return *that;
}
_PLATFORM T& Object3D_setTexture_(T* that, TextureLocalManager& tman, INT uID, INT t) {
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
_PLATFORM T& Object3D_setTextureD_(T* that, TextureManager& tman, INT uID, INT t) {
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
_PLATFORM DWORD Object3D_getTextureActual_(T* that, EFTYPE x, EFTYPE y) {
	if (NULL == that->texture) {
		return that->color;
	}
	if (that->t_w == 0 || that->t_h == 0) {
		return that->color;
	}
	INT _u = x, _v = y;
	_u += that->u;
	_v += that->v;
	_u %= that->t_w;
	_v %= that->t_h;
	if (_u < 0) {
		_u = that->t_w + _u;
	}
	if (_v < 0) {
		_v = that->t_h + _v;
	}
	return that->texture[_u + _v * that->t_w];
}
_PLATFORM DWORD Object3D_getTexture_(T* that, EFTYPE x, EFTYPE y) {
	if (NULL == that->texture) {
		return that->color;
	}
	if (that->t_w == 0 || that->t_h == 0) {
		return that->color;
	}
	INT _u = x * that->t_w;
	INT _v = y * that->t_h;
	_u += that->u;
	_v += that->v;
	_u %= that->t_w;
	_v %= that->t_h;
	if (_u < 0) {
		_u = that->t_w + _u;
	}
	if (_v < 0) {
		_v = that->t_h + _v;
	}
	return that->texture[_u + _v * that->t_w];
}
_PLATFORM DWORD Object3D_getTextureColor_(T* that, Vert3D& n0, Vert3D& n1, Vert3D& n2, Vert3D& n3, VObj* v, VObj* v0, VObj* v1, Vert3D* v_n) {
	if (NULL == that->cam) {
		return that->color;
	}
	Object3D* obj = (Object3D*)that;
	Camera3D* cam = (Camera3D*)that->cam;
	DWORD color;
	DWORD* __image = &color;
	if (obj->texture_type == 0) {
		n2.set(n1)* obj->M_1;
		if (v_n) {
			v_n->set(v->n_r);
		}
		EFTYPE sxy = n3.set(0, 0, 1) ^ (v->n);
		EFTYPE syz = n3.set(1, 0, 0) ^ (v->n);
		EFTYPE sxz = n3.set(0, 1, 0) ^ (v->n);
		EFTYPE mx = v->aabb[0].x - v->aabb[1].x;
		EFTYPE my = v->aabb[0].y - v->aabb[1].y;
		EFTYPE mz = v->aabb[0].z - v->aabb[1].z;
		if (sxy < 0) sxy = -sxy;
		if (syz < 0) syz = -syz;
		if (sxz < 0) sxz = -sxz;
		if (mx < 0) mx = -mx;
		if (my < 0) my = -my;
		if (mz < 0) mz = -mz;
		if (sxy > sxz) {
			if (sxy > syz) {
				*__image = obj->getTexture(obj, n2.x / mx, n2.y / my);
			}
			else {
				*__image = obj->getTexture(obj, n2.y / my, n2.z / mz);
			}
		}
		else {
			if (sxz > syz) {
				*__image = obj->getTexture(obj, n2.x / mx, n2.z / mz);
			}
			else {
				*__image = obj->getTexture(obj, n2.y / my, n2.z / mz);
			}
		}
	}
	else if (obj->texture_type == 1) {
		n2.set(0, 0, 0, 1);
		n2* obj->M* cam->M;
		n3.set(n0);
		n3 - n2;

		if (v_n) {
			v_n->set(n3);
		}

		n2.set(n0);
		EFTYPE cross = n2 ^ n3;
		n3* (cross * 2);
		n2 - n3;
		n3.set(cam->lookat);
		n2 + n3;
		n2.normalize();

		n2.x = n2.x * 0.5 + 0.5;
		n2.y = n2.y * 0.5 + 0.5;

		*__image = obj->getTexture(obj, n2.x, n2.y);
	}
	else if (obj->texture_type == 2) {
		n2.set(0, 0, 0, 1);
		n3.set(n1)* obj->M_1;
		n3 - n2;

		if (v_n) {
			v_n->set(n3)* obj->M* cam->M;
		}

		n2.set(n1)* obj->M_1;
		EFTYPE cross = n2 ^ n3;
		n3* (cross * 2);
		n2 - n3;
		n3.set(cam->lookat);
		n2 + n3;
		n2.normalize();

		n2.x = n2.x * 0.5 + 0.5;
		n2.y = n2.y * 0.5 + 0.5;

		*__image = obj->getTexture(obj, n2.x, n2.y);

	}
	else if (obj->texture_type == 3) {
		n2.set(0, 0, 0, 1);
		n2* obj->M;
		n3.set(n1);
		n3 - n2;

		if (v_n) {
			v_n->set(n3)* cam->M;
		}

		n2.set(n1);
		EFTYPE cross = n2 ^ n3;
		n3* (cross * 2);
		n2 - n3;
		n3.set(cam->lookat);
		n2 + n3;
		n2.normalize();

		n2.x = n2.x * 0.5 + 0.5;
		n2.y = n2.y * 0.5 + 0.5;

		*__image = obj->getTexture(obj, n2.x, n2.y);

	}
	else if (obj->texture_type == 4) {
		n2.set(0, 0, 0, 1);
		n2* obj->M;
		n3.set(n1);
		n3 - n2;

		if (v_n) {
			v_n->set(n3)* cam->M;
		}

		EFTYPE sxy = n2.set(0, 0, 1) ^ n3, syz = n2.set(1, 0, 0) ^ n3, sxz = n2.set(0, 1, 0) ^ n3;
		n2.set(n1);
		EFTYPE cross = n2 ^ n3;
		n3* (cross * 2);
		n2 - n3;
		n3.set(cam->lookat);
		n2 + n3;
		n2.normalize();

		n2.x = n2.x * 0.5 + 0.5;
		n2.y = n2.y * 0.5 + 0.5;
		n2.z = n2.z * 0.5 + 0.5;

		EFTYPE _sxy = sxy, _syz = syz, _sxz = sxz;
		if (sxy < 0) sxy = -sxy;
		if (syz < 0) syz = -syz;
		if (sxz < 0) sxz = -sxz;
		EFTYPE dw = 1.0 / 4.0, dh = 1.0 / 3.0;
		EFTYPE _dw = dw * obj->t_w, _dh = dh * obj->t_h;
		if (sxy > sxz) {
			if (sxy > syz) {
				if (_sxy < 0) {
					*__image = obj->getTextureActual(obj, _dw - (n2.x * _dw) + 2 * _dw, _dh - (n2.y * _dh) + 1 * _dh);
				}
				else {
					*__image = obj->getTextureActual(obj, n2.x * _dw + 0 * _dw, _dh - (n2.y * _dh) + 1 * _dh);
				}
			}
			else {
				if (_syz < 0) {
					*__image = obj->getTextureActual(obj, n2.z * _dh + 1 * _dw, _dh - (n2.y * _dw) + 1 * _dh);
				}
				else {
					*__image = obj->getTextureActual(obj, _dw - (n2.z * _dh) + 3 * _dw, _dh - (n2.y * _dw) + 1 * _dh);
				}
			}
		}
		else {
			if (sxz > syz) {
				if (_sxz < 0) {
					*__image = obj->getTextureActual(obj, _dw - (n2.x * _dw) + 2 * _dw, n2.z * _dh + 0 * _dh);
				}
				else {
					*__image = obj->getTextureActual(obj, _dw - (n2.x * _dw) + 2 * _dw, n2.z * _dh + 2 * _dh);
				}
			}
			else {
				if (_syz < 0) {
					*__image = obj->getTextureActual(obj, n2.z * _dh + 1 * _dw, _dh - (n2.y * _dw) + 1 * _dh);
				}
				else {
					*__image = obj->getTextureActual(obj, _dw - (n2.z * _dh) + 3 * _dw, _dh - (n2.y * _dw) + 1 * _dh);
				}
			}
		}
	}
	else if (obj->texture_type == 5) {

		n3.set(n1)* obj->M_1;

		EFTYPE _a = ((n3.x - v->v.x) * (v1->v.y - v->v.y) - (n3.y - v->v.y) * (v1->v.x - v->v.x)) /
			((v0->v.x - v->v.x) * (v1->v.y - v->v.y) - (v0->v.y - v->v.y) * (v1->v.x - v->v.x));
		EFTYPE _b = ((n3.x - v->v.x) * (v0->v.y - v->v.y) - (n3.y - v->v.y) * (v0->v.x - v->v.x)) /
			((v1->v.x - v->v.x) * (v0->v.y - v->v.y) - (v1->v.y - v->v.y) * (v0->v.x - v->v.x));
		EFTYPE pu = v->tu + _a * (v0->tu - v->tu) + _b * (v1->tu - v->tu);
		EFTYPE pv = v->tv + _a * (v0->tv - v->tv) + _b * (v1->tv - v->tv);

		n2.x = pu;
		n2.y = pv;

		*__image = obj->getTexture(obj, n2.x, n2.y);
	}
	return color;
}
_PLATFORM T& Object3D_setCenter_(T* that, EFTYPE x, EFTYPE y, EFTYPE z) {
	that->center.set(x, y, z);

	return *that;
}
_PLATFORM T& Object3D_addIndiceN_(T* that, EFTYPE x, EFTYPE y, EFTYPE z, EFTYPE nx, EFTYPE ny, EFTYPE nz) {
	VObj* v = that->indice.vobjPool->get(that->indice.vobjPool);
	if (!v) {
		return *that;
	}
	_VObj(v, x, y, z);
	if (!EP_ISZERO(nx + ny + nz)) {
		v->n.set(nx, ny, nz);
	}
	that->indice.insertLink(&that->indice, v, NULL, NULL);

	return *that;
}
_PLATFORM T& Object3D_addIndice_(T* that, EFTYPE x, EFTYPE y, EFTYPE z) {
	return that->addIndiceN(that, x, y, z, 0, 0, 0);
}
_PLATFORM T& Object3D_setIndiceI_(T* that, INT pv, INT pv0, INT pv1, INT anti_n) {
	if (that->indice.linkcount < 3) {
		return *that;
	}
	VObj* _v = that->indice.get(&that->indice, pv);
	VObj* _v0 = that->indice.get(&that->indice, pv0);
	VObj* _v1 = that->indice.get(&that->indice, pv1);

	if (NULL != _v && NULL != _v0 && NULL != _v1) {
		for (int i = 0; i < 3; i++) {
			VObj* __v = NULL;
			if (i == 0) {
				__v = _v0;
			}
			else if (i == 1) {
				__v = _v1;
			}
			else {
				__v = _v;
			}

			if (anti_n < 0) {
				that->addVertN(that, __v->v.x, __v->v.y, __v->v.z, -__v->n.x, -__v->n.y, -__v->n.z);
			}
			else {
				that->addVertN(that, __v->v.x, __v->v.y, __v->v.z, __v->n.x, __v->n.y, __v->n.z);
			}
		}
	}

	return *that;
}
_PLATFORM T& Object3D_setIndice_(T* that, INT pv, INT pv0, INT pv1) {
	return that->setIndiceI(that, pv, pv0, pv1, 0);
}
_PLATFORM T& Object3D_setLineColor_(T* that, COLORREF line) {
	that->line = line;

	return *that;
}
_PLATFORM T& Object3D_setColor_(T* that, COLORREF color) {
	that->color = color;

	return *that;
}
_PLATFORM T& Object3D_setColorB_(T* that, BYTE* c) {
	DWORD color = EGERGBA(c[0], c[1], c[2], 1);
	that->color = color;

	return *that;
}
_PLATFORM T& Object3D_addVertA_(T* that, EFTYPE x, EFTYPE y, EFTYPE z, INT anti) {
	VObj* v = that->verts.vobjPool->get(that->verts.vobjPool);
	if (!v) {
		return *that;
	}
	_VObj(v, x, y, z);
	int _anti = that->anti;
	that->anti *= anti;
	that->addVertI(that, v);
	that->anti = _anti;
	return *that;
}
_PLATFORM T& Object3D_addVert_(T* that, EFTYPE x, EFTYPE y, EFTYPE z) {
	VObj* v = that->verts.vobjPool->get(that->verts.vobjPool);
	if (!v) {
		return *that;
	}
	_VObj(v, x, y, z);
	return that->addVertI(that, v);
}
_PLATFORM T& Object3D_addVertN_(T* that, EFTYPE x, EFTYPE y, EFTYPE z, EFTYPE nx, EFTYPE ny, EFTYPE nz) {
	VObj* v = that->verts.vobjPool->get(that->verts.vobjPool);
	if (!v) {
		return *that;
	}
	_VObj(v, x, y, z);
	if (!EP_ISZERO(nx + ny + nz)) {
		v->n.set(nx, ny, nz);
	}
	return that->addVertI(that, v);
}
_PLATFORM T& Object3D_addVertI_(T* that, VObj* v) {
	if (NULL == v) {
		return *that;
	}

	if (that->v0 && that->v1) {
		if (EP_ISZERO(v->n.x + v->n.y + v->n.z)) {
			v->n.set(that->v0->v) - v->v;
			v->n_r.set(that->v1->v) - v->v;
			v->n* v->n_r;
			v->n.normalize();
		}
		for (int i = 0; i < 3; i++) {
			VObj* _v = NULL;
			if (i == 0) {
				_v = that->v0;
			}
			else if (i == 1) {
				_v = that->v1;
			}
			else {
				_v = v;
			}
			if (v->aabb[0].x < _v->v.x) v->aabb[0].x = _v->v.x;
			if (v->aabb[0].y < _v->v.y) v->aabb[0].y = _v->v.y;
			if (v->aabb[0].z < _v->v.z) v->aabb[0].z = _v->v.z;

			if (v->aabb[1].x > _v->v.x) v->aabb[1].x = _v->v.x;
			if (v->aabb[1].y > _v->v.y) v->aabb[1].y = _v->v.y;
			if (v->aabb[1].z > _v->v.z) v->aabb[1].z = _v->v.z;

			if (that->vertex_type == 1 || that->vertex_type == 2) {
				if (EP_ISZERO(_v->n.x + _v->n.y + _v->n.z)) {
					_v->n.set(v->n);
				}
			}
		}
		if (that->vertex_type == 1) {
			that->v0 = NULL;
			that->v1 = NULL;
		}
		else if (that->vertex_type == 2) {
			if ((that->verts.linkcount + 2) % 2 == 0) {
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

	if (that->render_aabb) {
		if (that->rightBottomFront_O.x < v->v.x) that->rightBottomFront_O.x = v->v.x;
		if (that->rightBottomFront_O.y < v->v.y) that->rightBottomFront_O.y = v->v.y;
		if (that->rightBottomFront_O.z < v->v.z) that->rightBottomFront_O.z = v->v.z;

		if (that->leftTopBack_O.x > v->v.x) that->leftTopBack_O.x = v->v.x;
		if (that->leftTopBack_O.y > v->v.y) that->leftTopBack_O.y = v->v.y;
		if (that->leftTopBack_O.z > v->v.z) that->leftTopBack_O.z = v->v.z;
	}

	that->verts.insertLink(&that->verts, v, NULL, NULL);

	if (that->anti < 0) {
		v->n.negative();
	}

	v->tu = that->u;
	v->tv = that->v;

	return *that;
}
_PLATFORM T& Object3D_renderAABB_(T* that) {
	that->render_aabb = 1;

	return *that;
}
_PLATFORM void Object3D_refreshAABBW_(T* that) {
	if (that->render_aabb) {
		for (int i = 0; i < 8; i++) {
			that->aabb_w[i].set(that->aabb[i])* that->M;
		}
	}
}
_PLATFORM void Object3D_refreshAABBC_(T* that) {
	if (that->render_aabb) {
		for (int i = 0; i < 8; i++) {
			that->aabb_r[i].set(that->aabb_w[i])* that->cam->M;
		}
	}
}
_PLATFORM void Object3D_render_normalize_(T* that, int mode) {
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
_PLATFORM void Object3D_shaderVertex_(T* that) {
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

				v->v_w.set(v->v)* that->CM;
				x0 = v->v_w.x;
				y0 = v->v_w.y;
				z0 = v->v_w.z;

				v->n_w.set(v->n)* that->CM;
				v->n_w.normalize();

				nx = v->n_w.x;
				ny = v->n_w.y;
				nz = v->n_w.z;
				d = -(v->n_w ^ v->v_w);

				v->R.mx.set(1 - 2 * nx * nx, -2 * ny * nx, -2 * nz * nx, -2 * d * nx);
				v->R.my.set(-2 * nx * ny, 1 - 2 * ny * ny, -2 * nz * ny, -2 * d * ny);
				v->R.mz.set(-2 * nx * nz, -2 * ny * nz, 1 - 2 * nz * nz, -2 * d * nz);
				v->R.mw.set(0, 0, 0, 1);

				v->n_w.set(v->n).negative()* that->CM;
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

			v->v_c.set(v->v)* that->CM;
			v->zz = v->v_c.z;
			v->v_r.set(v->v_c);
			that->debugger = v->v_r.z;
			v->cut = !that->cam->normalize_cut(that->cam, *v, *that->v0, *that->v1);
			v->cut = !that->cam->normalize(that->cam, v->v_r);
			if (v->cut) {
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
					v->n_1_z* that->center_r;

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
				that->center_r.set(that->center)* that->CM;
				if (!that->cam->normalize(that->cam, that->center_r)) {
					that->center_r.x = that->center_r.x * that->cam->scale_w + that->cam->offset_w;
					that->center_r.y = that->center_r.y * that->cam->scale_h + that->cam->offset_h;
				}
			}
		}
	}
}
_PLATFORM T& Object3D_move_(T* that, EFTYPE dx, EFTYPE dy, EFTYPE dz) {
	that->_M.move(dx, dy, dz);

	that->render_normalize(that, 1);

	return *that;
}
_PLATFORM T& Object3D_scale_(T* that, EFTYPE sx, EFTYPE sy, EFTYPE sz) {
	that->_M.scale(sx, sy, sz);

	that->render_normalize(that, 1);

	return *that;
}
_PLATFORM T& Object3D_rotate_(T* that, EFTYPE ax, EFTYPE ay, EFTYPE az) {
	that->_M.rotate(ax, ay, az);

	that->render_normalize(that, 1);

	return *that;
}