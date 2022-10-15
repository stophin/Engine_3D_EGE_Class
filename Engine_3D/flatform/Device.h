//Device.h
//

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "../math3d/Manager3D.h"
#include "../raytracing/Ray.h"
#include "../scene/OctTree.h"

#include "../raytracing/hittable_list.h"
#include "../raytracing/sphere.h"
#include "../raytracing/camere.h"
#include "../raytracing/material.h"
#include "../raytracing/bvh.h"

#define MAX_LIGHT	10
#define MAX_OBJECT	100
#define MAX_PRECISE 100000.0

struct Device {
	INT width;
	INT height;

	EFTYPE *depth;//Depth test buffer
	DWORD *image;//Image buffer
	DWORD *tango;//Target buffer
	EFTYPE *shade[MAX_LIGHT];//Shade buffer
	DWORD *trans;//Transparent buffer
	EFTYPE *deptr;//Reflection depth buffer
	DWORD *miror;//Reflection bufer
	DWORD *raytracing;//Ray Tracing buffer

	INT draw_line = -1;//draw thread split line
	INT draw_oct = 0;//draw oct tree boundaries
	INT move_light = -1;//moving light mode
	INT move_trans = -1;//move object in z axis
	INT render_linear = -1;//render in wire frame
	INT render_proj = -1;//render projection
	INT render_light = -1;//render multiple light
	INT light_count = 0;//light shader count
	INT render_mirror = -1;//render mirror in rasteration mode
	INT render_raytracing = -1;//render raytracing
	INT thread_count = 10;//render raytracing thread count, single thread is minus 0
	INT thread_count_h = 10;//specify horizonal thread count, then vertical thread count is thread_count - thread_count_h
	INT raytracing_count = 2;//reflection/refraction times between reflection\refraction objects when raytracing
	INT thread_w = MAX_OBJECT;
	INT thread_h = MAX_OBJECT;
	INT render_thread = -1;
	INT threadImageCount = 0;
	DWORD **threadImage;
	INT threadRenderCount = 0;
	VObj *** threadRender;
	INT render_mode = 0;//0: ray casting 1:ray tracing

	struct RenderParameters{
		Manager3D * man;
		Device * device;
		HANDLE hMutex;
		HANDLE hThread;
		void * pool;
		INT sx;
		INT sy;
		INT ex;
		INT ey;
		INT id;
		void* objPool;
		void* vobjPool;
		void* recordsPool;
	};

	EPoint e, s, v, c;
	Vert3D p;
	Vert3D n, n_1, n_2, n0, n1, n2, n3, r;
	Vert3D _n0, _n1, _n2, _n3;
	DWORD * _tango, *_image, *_trans, *_mirror;
	EFTYPE * _depth;
	DWORD * __image, *__tango, *__trans, *__mirror;
	EFTYPE *__depth, *__shade;
	DWORD ___image, ___last = 0;

	Device() :
		width(100),
		height(100),
		depth(NULL),
		image(NULL),
		tango(NULL),
		//shade(NULL),
		trans(NULL),
		deptr(NULL),
		miror(NULL),
		raytracing(NULL),
		threadRender(NULL),
		threadImage(NULL),
		thread_ready(0),
		thread_ready_r(0){
		for (INT i = 0; i < MAX_LIGHT; i++) {
			shade[i] = NULL;
		}
	}
	Device(INT w, INT h){
		Resize(w, h);
	}
	~Device() {
		Destory();
	}

	void Destory() {
		EndAllThread();
		EndAllThread_R();
		if (depth) {
			delete[] depth;
			depth = NULL;
		}
		if (image) {
			delete[] image;
			image = NULL;
		}
		if (tango) {
			delete[] tango;
			tango = NULL;
		}
		if (shade) {
			for (INT i = 0; i < light_count; i++) {
				if (shade[i]) {
					delete[] shade[i];
					shade[i] = NULL;
				}
			}
		}
		if (trans) {
			delete[] trans;
			trans = NULL;
		}
		if (deptr) {
			delete[] deptr;
			deptr = NULL;
		}
		if (miror) {
			delete[] miror;
			miror = NULL;
		}
		if (raytracing) {
			delete[] raytracing;
			raytracing = NULL;
		}
	}

	void Resize(INT w, INT h)  {
		Destory();
		width = w;
		height = h;

		depth = new EFTYPE[width * height];
		image = new DWORD[width * height];
		tango = new DWORD[width * height];
		for (INT i = 0; i < light_count; i++) {
			shade[i] = new EFTYPE[width * height];
		}
		trans = new DWORD[width * height];
		deptr = new EFTYPE[width * height];
		miror = new DWORD[width * height];
		_image = image;
		_tango = tango;
		_trans = trans;
		_mirror = miror;
		_depth = depth;
		raytracing = new DWORD[width * height];
	}

	//must be called after depth was rendered
	void RenderMirror(Manager3D & man) {
		//switch to the reflection camera
		Cam3D * cur_cam = man.cams.link;
		Camera3D * cam = man.getCamera(1);
		if (NULL == cam || NULL == cur_cam) {
			return;
		}
		//set cam parameters to current camera
		cam->M.set(cur_cam->M);
		cam->M_1.set(cur_cam->M_1);

		Obj3D * obj = man.refl.link, *temp = NULL;

		DWORD *__tango;
		INT index = 0;
		EFTYPE z;
		INT line_state = 0;
		INT line_l = 0, line_r = 0;
		if (obj) {
			do {

				VObj * v = obj->verts_f.link;
				VObj * v0 = NULL, *v1 = NULL;
				// more than 3 verts
				if (v && obj->verts_f.linkcount >= 3 && obj->reflection > 0) {

					// do not refresh relection surfaces to aviod dead loop
					temp = man.refl.link;
					man.refl.link = NULL;
					INT traverseCount = 0;
					do {
						traverseCount++;
						//there must be three verts
						if (v0 && v1) {
							if (v->backface > 0) {

								// set camera matrix to vertex's reflection matrix
								cur_cam->M.set(cam->M) * v->R;
								cur_cam->M_1.set(v->R_r) * cam->M_1;
								man.refresh((Camera3D*)cur_cam);
								man.shaderVertex(NULL);

								// get reflection projection to array mirror
								// need to change target device and depth array
								DWORD * ___tango = _tango;
								_tango = _mirror;
								EFTYPE * ___depth = _depth;
								_depth = deptr;
								//memset(depth, 0, width * height * sizeof(EFTYPE));
								//clear reflection depth and drawing
								for (INT i = v->ys; i <= v->ye && i < height; i++) {
									for (INT j = v->xs; j <= v->xe && j < width; j++) {
										index = i * width + j;
										_depth[index] = EP_BLACK;
										_image[index] = EP_BLACK;
										_mirror[index] = EP_BLACK;
									}
								}
								Render(man, v, v0, v1);
								// restore target device and depth array
								_depth = ___depth;
								_tango = ___tango;

								Draw_Line(_image, width, height, v0->x0, v0->y0, v1->x0, v1->y0, WHITE);
								Draw_Line(_image, width, height, v1->x0, v1->y0, v->x0, v->y0, WHITE);
								Draw_Line(_image, width, height, v->x0, v->y0, v0->x0, v0->y0, WHITE);

								for (INT i = v->ys; i <= v->ye && i < height; i++) {
									//little trick^_^
									line_state = 0;
									line_l = 0; 
									line_r = 0;
									//trick: pre-judge
									for (INT j = v->xs; j <= v->xe && j < width; j += 1) {
										__image = &_image[i * width + j];
										//up pulse
										if (*__image != EP_BLACK) {
											line_state++;
											if (line_state == 1) {
												line_l = j;
											}
											else {//if (line_state == 2) {
												line_r = j;
											}
											*__image = EP_BLACK;
										}
									}
									EFTYPE view_h = (i - cam->offset_h) / cam->scale_h;
									for (INT j = line_l; j <= line_r && j < width; j++) {
										index = i * width + j;
										__mirror = &_mirror[index];
										__image = &_image[index];
										if (j >= line_l && j <= line_r) {
											*__image = obj->color;
										}
										if (*__image != EP_BLACK) {
											__depth = &_depth[index];

											// get depth
											//(-n.x * ((EFTYPE)j - v.x) - n.y * ((EFTYPE)i - v.y)) / n.z + v->z
											n0.set((j - cam->offset_w) / cam->scale_w, view_h, 0, 1);
											//z = Vert3D::getZ(v->n_d, v->x0, v->y0, v->z0, (EFTYPE)j, (EFTYPE)i);
											z = Vert3D::getZ(v->n_1_z, v->x, v->y, v->z, n0.x, n0.y);
											if (EP_ISZERO(*__depth)) {
												*__depth = z;
											}
											if (*__depth <= z) {
												*__depth = z;

												__tango = &_tango[index];
												//adding light reduction in reflection
												*__tango = Light3D_multi(*__mirror, 0.8);
											}
										}
										//clear reflection depth at the same time
										deptr[index] = EP_BLACK;
										//clear drawing
										*__mirror = EP_BLACK;
										*__image = EP_BLACK;
									}
								}
							}
							if (obj->vertex_type == 1) {
								v0 = NULL;
								v1 = NULL;
							}
							else if (obj->vertex_type == 2) {
								if ((traverseCount + 1) % 2 == 0) {
									v0 = v;
									//this->v1 = this->v1;
								}
								else {
									v0 = v1;
									v1 = v;
								}
							}
							else {
								v0 = v1;
								v1 = v;
							}
						}
						else if (v0 == NULL) {
							v0 = v;
						}
						else if (v1 == NULL) {
							v1 = v;
						}

						v = obj->verts_f.next(&obj->verts_f, v);
					} while (v && v != obj->verts_f.link);
					// do not refresh relection surfaces
					// so that there will be no loop
					// and the parameters which passed to
					// render() will not be changed
					man.refl.link = temp;
				}

				obj = man.refl.next(&man.refl, obj);
			} while (obj && obj != man.refl.link);
		}

		// restore current camera and refresh
		cur_cam->M.set(cam->M);
		cur_cam->M_1.set(cam->M_1);
		man.refresh(NULL);
		man.shaderVertex(NULL);
	}


	void RenderShade(Manager3D& man) {
		//switch to the shadow camera
		Cam3D * cur_cam = man.cams.link;
		Camera3D * cam = man.getCamera(1);
		if (NULL == cam || NULL == cur_cam) {
			return;
		}
		cam->M.set(cur_cam->M);
		cam->M_1.set(cur_cam->M_1);

		EFTYPE * _shade = NULL;
		for (INT i = 0; i < light_count; i++) {
			_shade = shade[i];
			memset(_shade, 0, width * height * sizeof(EFTYPE));
		}

		Lgt3D * lgt = man.lgts.link;
		INT shadeIndex = 0;
		if (lgt) {
			do{
				if (shadeIndex >= light_count) {
					break;
				}
				_shade = shade[shadeIndex++];
				Mat3D mm, mm_1;

				VObj * _range = NULL;

				Obj3D * obj = man.objs.link;

				EFTYPE z;
				INT index = 0, _index = 0;
				INT xs, xe, ys, ye;
				INT line_state = 0;
				INT line_l = 0, line_r = 0;
				if (obj) {
					//set camera matrix to anti-light matrix
					cur_cam->M.set(lgt->M_1);
					cur_cam->M_1.set(lgt->M);
					man.refresh((Camera3D*)cur_cam);

					INT render_state = 0;
					INT trans_w0 = EP_MAX, trans_h0 = EP_MAX;
					INT trans_w1 = -EP_MAX, trans_h1 = -EP_MAX;
					do {
						VObj * v = obj->verts_r.link;
						// more than 3 verts
						if (v && obj->verts_r.linkcount >= 3) {
							VObj *v0 = NULL, *v1 = NULL, *vtemp;
							INT traverseCount = 0;
							do {
								traverseCount++;
								//there must be three verts
								if (v0 && v1) {
									// back face cull
									// shade do not need backface culling?
									if (v->backface > 0)
									{

										_range = v;
										// in range
										if (_range) {
											//draw triangle contour
											Draw_Line(_image, width, height, v0->x0, v0->y0, v1->x0, v1->y0, WHITE);
											Draw_Line(_image, width, height, v1->x0, v1->y0, v->x0, v->y0, WHITE);
											Draw_Line(_image, width, height, v->x0, v->y0, v0->x0, v0->y0, WHITE);

											index = 0;
											xs = v->xs; xe = v->xe; ys = v->ys; ye = v->ye;
											for (INT i = ys; i <= ye && i < height; i++) {

												//little trick^_^
												line_state = 0;
												line_l = 0, line_r = 0;
												//trick: pre-judge
												for (INT j = xs; j <= xe && j < width; j += 1) {
													__image = &_image[i * width + j];
													//up pulse
													if (*__image != EP_BLACK) {
														line_state++;
														if (line_state == 1) {
															line_l = j;
														}
														else {//if (line_state == 2) {
															line_r = j;
														}
														*__image = EP_BLACK;
													}
												}
												EFTYPE view_h = (i - cam->offset_h) / cam->scale_h;
												for (INT j = line_l; j <= line_r && j < width; j++) {
													index = i * width + j;
													__image = &_image[index];
													if (j >= line_l && j <= line_r) {
														*__image = obj->color;
													}

													if (*__image != EP_BLACK) {
														__shade = &_shade[index];
														// get shade
														//(-n.x * ((EFTYPE)j - v.x) - n.y * ((EFTYPE)i - v.y)) / n.z + v->z
														n0.set((j - cam->offset_w) / cam->scale_w, view_h, 0, 1);
														//z = Vert3D::getZ(v->n_d, v->x0, v->y0, v->z0, (EFTYPE)j, (EFTYPE)i);
														z = Vert3D::getZ(v->n_1_z, v->x, v->y, v->z, n0.x, n0.y);
														if (EP_ISZERO(*__shade)) {
															*__shade = z;
														}
														if (*__shade <= z) {
															*__shade = z;
														}
													}
													*__image = EP_BLACK;
												}
											}
										}
									}

									if (obj->vertex_type == 1) {
										v0 = NULL;
										v1 = NULL;
									}
									else if (obj->vertex_type == 2) {
										if ((traverseCount + 1) % 2 == 0) {
											v0 = v;
											//this->v1 = this->v1;
										}
										else {
											v0 = v1;
											v1 = v;
										}
									}
									else {
										v0 = v1;
										v1 = v;
									}
								}
								else if (v0 == NULL) {
									v0 = v;
								}
								else if (v1 == NULL) {
									v1 = v;
								}

								v = obj->verts_r.next(&obj->verts_r, v);
							} while (v && v != obj->verts_r.link);
						}

						//first do objects till end
						//then do reflection and then transparent object
						if (render_state == 0) {
							obj = man.objs.next(&man.objs, obj);
							if (!(obj && obj != man.objs.link)) {
								obj = man.refl.link;
								render_state = 1;
								if (!obj) {
									//or render reflection points
									obj = man.tras.link;
									render_state = 2;
								}
							}
						}
						else if (render_state == 1) {
							obj = man.refl.next(&man.refl, obj);
							if (!(obj && obj != man.refl.link)) {
								obj = man.tras.link;
								render_state = 2;
							}
						}
						else {
							obj = man.tras.next(&man.tras, obj);
							if (!(obj && obj != man.tras.link)) {
								break;
							}
						}
					} while (obj);
				}
				if (render_light < 0) {
					break;
				}

				lgt = man.lgts.next(&man.lgts, lgt);
			} while (lgt && lgt != man.lgts.link);
		}

		// restore current camera and refresh
		cur_cam->M.set(cam->M);
		cur_cam->M_1.set(cam->M_1);
		man.refresh(NULL);
	}

	void ClearBeforeRender() {
		//memset(depth, 0, width * height * sizeof(EFTYPE));
		memset(tango, 0, width * height * sizeof(DWORD));
		memset(image, 0, width * height * sizeof(DWORD));
		memset(trans, 0, width * height * sizeof(DWORD));
	}
	void ClearBeforeRenderDepth() {
		memset(depth, 0, width * height * sizeof(EFTYPE));
	}

	HANDLE thread_main_r;
	HANDLE * thread_pool_r = NULL;
	HANDLE hMutex_r;
	RenderParameters * param_r = NULL;
	INT thread_ready_r;
	INT * thread_status_r = NULL;
	INT thread_all_done_r;
	INT thread_ready_count;

	void RenderThread(Manager3D& man) {
		if (0 == thread_ready_r) {
			//线程数 thread_count * thread_count_h
			//max is limit by MAX_OBJ3D_MAX macro
			//创建互斥体  
			hMutex_r = CreateMutex(NULL, FALSE, TEXT("Mutex_R"));
			//创建线程  
			INT dx = thread_w / thread_count_h;
			INT dy = thread_h / thread_count;
			if (thread_pool_r == NULL) {
				thread_pool_r = new HANDLE[thread_count_h * thread_count];
			}
			if (thread_status_r == NULL) {
				thread_status_r = new INT[thread_count_h * thread_count];
			}
			if (param_r == NULL) {
				param_r = new RenderParameters[thread_count_h * thread_count];
			}
			if (threadRender == NULL) {
				threadRender = new VObj**[thread_w * thread_h];
				for (INT i = 0; i < thread_w; i++) {
					for (INT j = 0; j < thread_h; j++) {
						threadRender[j * thread_w + i] = new VObj*[3];
						threadRender[j * thread_w + i][0] = NULL;
						threadRender[j * thread_w + i][1] = NULL;
						threadRender[j * thread_w + i][2] = NULL;
					}
				}
			}
			threadImageCount = thread_count_h * thread_count;
			if (threadImage == NULL) {
				threadImage = new DWORD*[threadImageCount];
				for (INT i = 0; i < threadImageCount; i++) {
					threadImage[i] = new DWORD[width * height];
				}
			}
			for (INT i = 0; i < thread_count_h; i++) {
				for (INT j = 0; j < thread_count; j++) {
					INT index = i * thread_count + j;

					param_r[index].man = &man;
					param_r[index].device = this;
					param_r[index].id = index;
					param_r[index].hMutex = hMutex_r;
					SetRect(param_r[index], dx * i, dy * j, dx * i + dx, dy * j + dy);

					thread_status_r[index] = 1;
					thread_pool_r[index] = CreateThread(NULL, 0, RenderThreadProc_R, &param_r[index], 0, NULL);
					param_r[index].hThread = thread_pool_r[index];
				}
			}
			thread_ready_r = 1;
		}
		else {
			for (INT i = 0; i < thread_count_h; i++) {
				for (INT j = 0; j < thread_count; j++) {
					INT index = i * thread_count + j;
					param_r[index].man = &man;
					param_r[index].device = this;

					thread_status_r[index] = 1;
					ResumeThread(thread_pool_r[index]);
				}
			}
		}
		if (!thread_main_r) {
			//最后启动主线程并运行
			thread_main_r = CreateThread(NULL, 0, RenderThreadMain_R, this, 0, NULL);
			//句柄不再用到就关闭
			CloseHandle(thread_main_r);
		}
		//调用的线程退出以进行后续显示工作
		return;
	}
	void EndAllThread_R() {
		//没有运行线程
		if (0 == thread_ready_r) {
			return;
		}
		//没有运行线程资源
		if (!thread_pool_r || !param_r || !thread_status_r) {
			return;
		}
		for (INT i = 0; i < thread_count_h; i++) {
			for (INT j = 0; j < thread_count; j++) {
				INT index = i * thread_count + j;
				//将这些参数设置为NULL以结束线程
				param_r[index].man = NULL;

				ResumeThread(thread_pool_r[index]);
			}
		}
		//等待线程退出  
		for (INT i = 0; i < thread_count_h; i++) {
			for (INT j = 0; j < thread_count; j++) {
				WaitForSingleObject(thread_pool_r[i * thread_count + j], INFINITE);
			}
		}

		//关闭句柄，释放资源  
		for (INT i = 0; i < thread_count_h; i++) {
			for (INT j = 0; j < thread_count; j++) {
				CloseHandle(thread_pool_r[i * thread_count + j]);
			}
		}
		//释放互斥资源
		CloseHandle(hMutex_r);
		//清除资源
		if (thread_pool_r) {
			delete[] thread_pool_r;
		}
		if (thread_status_r) {
			delete[] thread_status_r;
		}
		if (param_r) {
			delete[] param_r;
		}
		if (threadRender) {
			for (INT i = 0; i < thread_w; i++) {
				for (INT j = 0; j < thread_h; j++) {
					delete[] threadRender[j * thread_w + i];
				}
			}
			delete[] threadRender;
		}
		if (threadImage) {
			for (INT i = 0; i < threadImageCount; i++) {
				delete[] threadImage[i];
			}
			delete[] threadImage;
		}
	}
	INT IsThreadDone() {
		//没有运行线程
		if (0 == thread_ready_r) {
			return 1;
		}
		//没有运行线程资源
		if (!thread_pool_r || !param_r || !thread_status_r) {
			return 1;
		}
		//等待线程退出
		INT _thread_all_done = 1;
		for (INT i = 0; i < thread_count_h; i++) {
			for (INT j = 0; j < thread_count; j++) {
				INT index = i * thread_count + j;
				if (1 == thread_status_r[index]) {
					_thread_all_done = 0;
					break;
				}
			}
			if (0 == _thread_all_done) {
				break;
			}
		}
		return _thread_all_done;
	}
	static DWORD WINAPI RenderThreadMain_R(LPVOID lpThreadParameter) {
		Device * device = (Device*)lpThreadParameter;
		if (NULL == device) {
			return 0;
		}
		device->thread_all_done_r = 1;
		//等待线程退出
		INT _thread_all_done;
		while (1) {
			_thread_all_done = 1;
			for (INT i = 0; i < device->thread_count_h; i++) {
				for (INT j = 0; j < device->thread_count; j++) {
					INT index = i * device->thread_count + j;
					if (1 == device->thread_status_r[index]) {
						_thread_all_done = 0;
						break;
					}
				}
				if (0 == _thread_all_done) {
					break;
				}
			}
			if (_thread_all_done) {
				break;
			}
		}
		device->thread_all_done_r = 0;
		return 0;
	}
	static DWORD WINAPI RenderThreadProc_R(LPVOID lpThreadParameter) {
		RenderParameters * pthread = (RenderParameters*)lpThreadParameter;
		if (NULL == pthread) {
			return 0;
		}
		while (1) {
			if (NULL == pthread->man) {
				break;
			}
			RenderThreadSub(*pthread->man, pthread->sx, pthread->sy, pthread->ex, pthread->ey, pthread->id, pthread->hMutex, pthread->device);
			pthread->device->thread_status_r[pthread->id] = 0;
			SuspendThread(pthread->hThread);
		}
		return 0;
	}
	void RenderThreadReady(Manager3D& man,VObj* range, VObj* range0, VObj* range1, Device* device) {
		VObj * _range;

		Mat3D mm;

		Obj3D * obj = man.objs.link;
		threadRenderCount = 0;
		INT dx = thread_w / thread_count_h;
		INT dy = thread_h / thread_count;
		INT thi = 0;
		INT thj = 0;
		INT othi = 0;
		INT othj = 0;
		if (obj) {
			INT render_state = 0;
			INT trans_w0 = EP_MAX, trans_h0 = EP_MAX;
			INT trans_w1 = -EP_MAX, trans_h1 = -EP_MAX;
			VObj * v, *v0, *v1, *vtemp;
			EPoint l1, l0, l;

			EFTYPE z;
			INT index = 0, _index = 0;
			INT xs, xe, ys, ye;
			INT i, j;
			INT res;
			Camera3D* cam = NULL;
			Lgt3D * lgt;
			EFTYPE zz;
			EFTYPE f, t, transparent, _i, _j;
			INT line_state = 0;
			INT line_l = 0, line_r = 0;
			INT inrange;

			DWORD* _image = device->threadImage[0];
			memset(_image, EP_BLACK, sizeof(DWORD) * device->width * device->height);
			//DWORD *_image = device->_image;
			EFTYPE* _depth = device->_depth;
			DWORD  ___image;
			EFTYPE depth;
			EFTYPE* __depth;
			EFTYPE* __shade;
			DWORD* __image;
			DWORD* __tango, * __trans;
			Vert3D n0, n1, n2, n3, r;
			Vert3D _n0, _n1, _n2, _n3;
			do {
				if (threadRenderCount >= thread_w * thread_h) {
					break;
				}
				v = obj->verts_r.link;
				// more than 3 verts
				if (v && obj->verts_r.linkcount >= 3) {
					v0 = NULL; v1 = NULL;
					INT traverseCount = 0;
					do {
						traverseCount++;
						//there must be three verts
						if (v0 && v1) {
							// back face culling
							if (v->backface > 0)
							{

								_range = NULL;
								if (range) {
									_range = range;
								}
								else {
									_range = v;
								}
								// in range
								inrange = true;
								if (_range != v) {
									inrange = EPoint::RectIsIntersect(v->xs, v->ys, v->xe, v->ye, _range->xs, _range->ys, _range->xe, _range->ye);
									//inrange = Vert3D::CrossRect(v->xs, v->ys, v->xe, v->ye, _range->xs, _range->ys, _range->xe, _range->ye);
								}
								if (_range && inrange) 
								{
									if (threadRenderCount >= thread_w * thread_h) {
										break;
									}
									threadRender[threadRenderCount][0] = v0;
									threadRender[threadRenderCount][1] = v1;
									threadRender[threadRenderCount][2] = v;
									threadRenderCount++;
									v->obj = (void*)obj;

									if (true) {
										//step1: render the triangle
										index = 0;
										xs = v->xs; xe = v->xe; ys = v->ys; ye = v->ye;
										//xs = _range == v ? v->xs : max(_range->xs, v->xs); ys = _range == v ? v->ys : max(_range->ys, v->ys);
										//xe = _range == v ? v->xe : min(_range->xe, v->xe); ye = _range == v ? v->ye : min(_range->ye, v->ye);
										//draw triangle contour
										device->Draw_Line(_image, device->width, device->height, v0->x0, v0->y0, v1->x0, v1->y0, WHITE);
										device->Draw_Line(_image, device->width, device->height, v1->x0, v1->y0, v->x0, v->y0, WHITE);
										device->Draw_Line(_image, device->width, device->height, v->x0, v->y0, v0->x0, v0->y0, WHITE);

										//get line formula
										//v0-v1
										Vert3D::GetLine(v1->v_s, v0->v_s, l1);
										//v1-v
										Vert3D::GetLine(v->v_s, v1->v_s, l);
										//v-v0
										Vert3D::GetLine(v0->v_s, v->v_s, l0);

										EFTYPE zz_f = (v->n_r.x * v->v_c.x + v->n_r.y * v->v_c.y + v->n_r.z * v->v_c.z);
										for (i = ys; i <= ye && i < device->height; i += 1) {
											cam = obj->cam;
											if (cam == NULL) {
												break;
											}
											//little trick^_^
											line_state = 0;
											line_l = 0, line_r = 0;
											if (false && device->render_linear < 0) {
												line_l = xs;
												line_r = xe;
											}
											else {
												//trick: pre-judge
												for (j = xs; j <= xe && j < device->width; j += 1) {
													__image = &_image[i * device->width + j];
													//up pulse
													if (*__image != EP_BLACK) {
														line_state++;
														if (line_state == 1) {
															line_l = j;
														}
														else {//if (line_state == 2) {
															line_r = j;
														}
														*__image = EP_BLACK;
													}
												}
											}
											//get range x
											EFTYPE __y = i;
											EFTYPE __x;
											INT _line_l1 = (INT)(l1.x * __y + l1.y);
											INT _line_l = (INT)(l.x * __y + l.y);
											INT _line_l0 = (INT)(l0.x * __y + l0.y);
											EFTYPE view_h = (i - cam->offset_h) / cam->scale_h;
											for (j = line_l; j <= line_r && j < device->width; j += 1) {
												index = i * device->width + j;
												__image = &_image[index];
												if (device->render_linear < 0) {
													if (j == line_l || j == line_r) {
														*__image = obj->color;
													}
												}
												else {
													if (j >= line_l && j <= line_r) {
														*__image = obj->color;
													}
												}
												//step2: depth test
												if (*__image != EP_BLACK) {
													// get depth
													//(-n.x * ((EFTYPE)j - v.x) - n.y * ((EFTYPE)i - v.y)) / n.z + v->z
													n0.set((j - cam->offset_w) / cam->scale_w, view_h, 0, 1);
													//z = Vert3D::getZ(v->n_d, v->x0, v->y0, v->z0, (EFTYPE)j, (EFTYPE)i);
													z = Vert3D::getZ(v->n_1_z, v->x, v->y, v->z, n0.x, n0.y);
													z *= MAX_PRECISE;
													__depth = &_depth[index];
													if (EP_ISZERO(*__depth)) {
														*__depth = z;
													}
													if (*__depth <= z) {
														*__depth = z;
													}

													if (device->render_linear < 0) {
														_image[index] = EP_BLACK;
													}
													else {
														_image[index] = EP_BLACK;
													}
												}
											}
										}
									}
								}
							}

							if (obj->vertex_type == 1) {
								v0 = NULL;
								v1 = NULL;
							}
							else if (obj->vertex_type == 2) {
								if ((traverseCount + 1) % 2 == 0) {
									v0 = v;
									//this->v1 = this->v1;
								}
								else {
									v0 = v1;
									v1 = v;
								}
							}
							else {
								v0 = v1;
								v1 = v;
							}
						}
						else if (v0 == NULL) {
							v0 = v;
						}
						else if (v1 == NULL) {
							v1 = v;
						}

						v = obj->verts_r.next(&obj->verts_r, v);
					} while (v && v != obj->verts_r.link);
				}


				//first do objects till end
				//then do reflection and then transparent object
				if (render_state == 0) {
					obj = man.objs.next(&man.objs, obj);
					if (!(obj && obj != man.objs.link)) {
						//obj = man.tras.link;
						////do not render reflection points
						//render_state = 2;
						obj = man.refl.link;
						render_state = 1;
						if (!obj) {
							obj = man.tras.link;
							render_state = 2;
						}
					}
				}
				else if (render_state == 1) {
					obj = man.refl.next(&man.refl, obj);
					if (!(obj && obj != man.refl.link)) {
						obj = man.tras.link;
						render_state = 2;
					}

				}
				else {
					obj = man.tras.next(&man.tras, obj);
					if (!(obj && obj != man.tras.link)) {
						//render transparent after all transparent objects were done
						break;
					}
				}
			} while (obj);
		}
		//printf("Got %d of %d\n", threadRenderCount, thread_w* thread_h);
		device->thread_ready_count = threadRenderCount;
	}
	//保证操作的原子性
#define THREAD_MUTEX_GET() //\
	if (hMutex) { WaitForSingleObject(hMutex, 1000); }
#define THREAD_MUTEX_RELEASE()//\
	if (hMutex) { ReleaseMutex(hMutex); }

	static void RenderThreadSub(Manager3D& man, INT sx, INT sy, INT ex, INT ey, INT id, HANDLE hMutex, Device* device = NULL) {
		VObj * _range;
		VObj* range = NULL;
		VObj* range0 = NULL;
		VObj* range1 = NULL;

		Mat3D mm;

		INT threadRenderCount = 0;
		INT renderIndexX = 0;
		INT renderIndexY = 0;
			INT render_state = 0;
			INT trans_w0 = EP_MAX, trans_h0 = EP_MAX;
			INT trans_w1 = -EP_MAX, trans_h1 = -EP_MAX;
			VObj * v = NULL, *v0 = NULL, *v1 = NULL, *vtemp;
			EPoint l1, l0, l;

			EFTYPE z;
			INT index = 0, _index = 0;
			INT xs, xe, ys, ye;
			INT i, j;
			INT res;
			Camera3D* cam = NULL;
			Lgt3D * lgt;
			EFTYPE zz;
			EFTYPE f, t, transparent, _i, _j;
			INT line_state = 0;
			INT line_l = 0, line_r = 0;
			INT inrange;

			if (id >= device->threadImageCount) {
				return;
			}
			DWORD *_image = device->threadImage[id];
			//printf("%d: %p\n", id, _image);
			memset(_image, EP_BLACK, sizeof(DWORD)* device->width * device->height);
			//DWORD *_image = device->_image;
			EFTYPE *_depth = device->_depth;
			DWORD  ___image;
			EFTYPE depth;
			EFTYPE * __depth;
			EFTYPE *__shade;
			DWORD * __image;
			DWORD * __tango, *__trans;
			Vert3D n0, n1, n2, n3, r;
			Vert3D _n0, _n1, _n2, _n3;
			Obj3D * obj = NULL;
			for (renderIndexY = sy; renderIndexY < ey; renderIndexY++) {
				if (threadRenderCount >= device->thread_ready_count) {
					break;
				}
				for (renderIndexX = sx; renderIndexX < ex; renderIndexX++) {
					threadRenderCount = renderIndexY * device->thread_w + renderIndexX;
					if (threadRenderCount >= device->thread_ready_count) {
						break;
					}
					v0 = device->threadRender[threadRenderCount][0];
					v1 = device->threadRender[threadRenderCount][1];
					v = device->threadRender[threadRenderCount][2];

					if (!v || !v0 || !v1) {
						continue;
					}
					obj = (Obj3D*)v->obj;
					if (!obj) {
						continue;
					}

					//step1: render the triangle
					index = 0;
					xs = v->xs; xe = v->xe; ys = v->ys; ye = v->ye;
					//xs = _range == v ? v->xs : max(_range->xs, v->xs); ys = _range == v ? v->ys : max(_range->ys, v->ys);
					//xe = _range == v ? v->xe : min(_range->xe, v->xe); ye = _range == v ? v->ye : min(_range->ye, v->ye);
					//draw triangle contour
					if (false) {
						device->Draw_Line(device->tango, device->width, device->height, v0->x0, v0->y0, v1->x0, v1->y0, BLUE);// (DWORD)_image);
						device->Draw_Line(device->tango, device->width, device->height, v1->x0, v1->y0, v->x0, v->y0, BLUE);//(DWORD)_image);
						device->Draw_Line(device->tango, device->width, device->height, v->x0, v->y0, v0->x0, v0->y0, BLUE);//(DWORD)_image);
						//continue;
					}
					device->Draw_Line(_image, device->width, device->height, v0->x0, v0->y0, v1->x0, v1->y0, WHITE);
					device->Draw_Line(_image, device->width, device->height, v1->x0, v1->y0, v->x0, v->y0, WHITE);
					device->Draw_Line(_image, device->width, device->height, v->x0, v->y0, v0->x0, v0->y0, WHITE);

					//get line formula
					//v0-v1
					Vert3D::GetLine(v1->v_s, v0->v_s, l1);
					//v1-v
					Vert3D::GetLine(v->v_s, v1->v_s, l);
					//v-v0
					Vert3D::GetLine(v0->v_s, v->v_s, l0);

					EFTYPE zz_f = (v->n_r.x * v->v_c.x + v->n_r.y * v->v_c.y + v->n_r.z * v->v_c.z);
					for (i = ys; i <= ye && i < device->height; i += 1) {
						cam = obj->cam;
						if (cam == NULL) {
							break;
						}
						//little trick^_^
						line_state = 0;
						line_l = xs, line_r = xe;
						if (false && device->render_linear < 0) {
							line_l = xs;
							line_r = xe;
						}
						else {
							//trick: pre-judge
							for (j = xs; j <= xe && j < device->width; j += 1) {
								__image = &_image[i * device->width + j];
								//up pulse
								if (*__image != EP_BLACK) {
									line_state++;
									if (line_state == 1) {
										line_l = j;
									}
									else {//if (line_state == 2) {
										line_r = j;
									}
									*__image = EP_BLACK;
								}
							}
						}
						//get range x
						EFTYPE __y = i;
						EFTYPE __x;
						INT _line_l1 = (INT)(l1.x * __y + l1.y);
						INT _line_l = (INT)(l.x * __y + l.y);
						INT _line_l0 = (INT)(l0.x * __y + l0.y);
						EFTYPE view_h = (i - cam->offset_h) / cam->scale_h;
						for (j = line_l; j <= line_r && j < device->width; j += 1) {
							index = i * device->width + j;
							__image = &_image[index];
							if (device->render_linear < 0) {
								if (j == line_l || j == line_r) {
									*__image = obj->color;
								}
							}
							else {
								if (j >= line_l && j <= line_r) {
									*__image = obj->color;
								}
							}
							//step2: depth test
							if (*__image != EP_BLACK) {
								// get depth
								//(-n.x * ((EFTYPE)j - v.x) - n.y * ((EFTYPE)i - v.y)) / n.z + v->z
								n0.set((j - cam->offset_w) / cam->scale_w, view_h, 0, 1);
								//z = Vert3D::getZ(v->n_d, v->x0, v->y0, v->z0, (EFTYPE)j, (EFTYPE)i);
								z = Vert3D::getZ(v->n_1_z, v->x, v->y, v->z, n0.x, n0.y);
								zz =z *  MAX_PRECISE;
								THREAD_MUTEX_GET();
								__depth = &_depth[index];
								if (EP_ISZERO(*__depth)) {
									//*__depth = z;
								}
								else if ((int)*__depth == (int)zz) {
								//else if (EP_GEZERO(zz - *__depth)) {
								//if (*__depth <= z) {
									//*__depth = z;
									__tango = &device->_tango[index];
									__trans = &device->_trans[index];

									//step3: render light
									//n0.set((j - cam->offset_w) / cam->scale_w, (i - cam->offset_h) / cam->scale_h, z, 1);
									n0.z = z;
									// get position
									n0 * cam->proj_1;
									zz =  zz_f / (v->n_r.x * n0.x + v->n_r.y * n0.y + v->n_r.z);
									n0.x *= zz;
									n0.y *= zz;
									n0.z = zz;
									n0.w = 1;
									n1.set(n0)* cam->M_1;

									*__image = obj->getTextureColor(obj, n0, n1, n2, n3, v, NULL);

									//get interpolation normal vector from 3 points of a triangle
									__x = j;
									if (1 == obj->normal_type) {
										Object3D_GetInterpolationNormalVector(v0, v1, v, __x, __y,
											line_r, line_l, _line_l1, _line_l, _line_l0,
											5, _n0, _n1, _n2, _n3);
									}
									else {
										_n0.set(v->n_r);
									}

									//calculate sumption of light factors
									lgt = man.lgts.link;
									f = 0;
									if (lgt) {
										do {
											f += lgt->getFactor(lgt, _n0, n0);

											if (device->render_light < 0) {
												break;
											}

											lgt = man.lgts.next(&man.lgts, lgt);
										} while (lgt && lgt != man.lgts.link);
									}

									//step4: render transparent
									if (!EP_ISZERO(obj->transparent)) {
										r.set(n0);
										//t = r.negative() & v->n_r;
										t = r.negative() & _n0;

										if (t < 0) t = -t;
										transparent = 1.0 / obj->transparent;
										if (transparent < 0) transparent = -transparent;

										_i = (i - obj->center_r.y) * (transparent / t) + obj->center_r.y;
										_j = (j - obj->center_r.x) * (transparent / t) + obj->center_r.x;

										if (obj->transparent < 0) {
											_i = 2 * obj->center_r.y - _i;
											_j = 2 * obj->center_r.x - _j;
										}
										if (!(_i < 0 || _i > device->height - 1 || _j < 0 || _j > device->width - 1)) {
											_index = (INT)_i * device->width + (INT)_j;
											//if (depth[_index] < z) 

											//*__trans = Light3D_multi(*__image, f);
											*__trans = Light3D_add(*__image, device->_tango[_index], f);

											if (*__trans == EP_BLACK) {
												//*__trans++;
												*__trans = *__image;
											}
											//get transparent range
											if (trans_w1 < j) trans_w1 = j;
											if (trans_h1 < i) trans_h1 = i;
											if (trans_w0 > j) trans_w0 = j;
											if (trans_h0 > i) trans_h0 = i;
										}
									}
									else {
										*__image = Light3D_multi(*__image, f);
									}
									*__tango = *__image;

									//step5: render shadow map
									lgt = man.lgts.link;
									INT shadeIndex = 0;
									EFTYPE * _shade = NULL;

									if (lgt) {
										do {
											if (shadeIndex >= device->light_count) {
												break;
											}
											_shade = device->shade[shadeIndex++];

											n2.set(n1) * lgt->M_1;
											cam->project(cam, n2);
											_j = (int)(n2.x * cam->scale_w + cam->offset_w), _i = (int)(n2.y * cam->scale_h + cam->offset_h);

											if (!(_i < 0 || _i > device->height - 1 || _j < 0 || _j > device->width - 1)) {
												_index = _i * device->width + _j;

												__shade = &_shade[_index];
												if (device->render_proj > 0) {
													device->_tango[_index] = RED;// obj->color;
												}

												//shadow
												if (EP_GTZERO(*__shade - n2.z - 1e-1)) {
													*__tango = Light3D_multi(*__tango, f / 5);
												}
											}

											lgt = man.lgts.next(&man.lgts, lgt);
										} while (lgt && lgt != man.lgts.link);
									}

									if (device->render_proj > 0) {
										n2.set(n0)*cam->M_1 *obj->M_1* obj->M* cam->M;
										//n2.set(n1) * cam->M;
										cam->project(cam, n2);

										INT __j = (int)(n2.x * cam->scale_w + cam->offset_w), __i = (int)(n2.y * cam->scale_h + cam->offset_h);
										//Draw_Line(_tango, width, height, __j, __i, _j, _i, RED);
										_j = __j, _i = __i;

										if (!(_i < 0 || _i > device->height - 1 || _j < 0 || _j > device->width - 1)) {
											_index = _i * device->width + _j;
											device->_tango[_index] = BLUE;// obj->color
										}
									}
								}
								THREAD_MUTEX_RELEASE();

								if (device->render_linear < 0) {
									_image[index] = EP_BLACK;
								}
								else {
									_image[index] = EP_BLACK;
								}
							}
						}
					}
				}
				//render transparent after all transparent objects were done
				index = 0;
				for (i = trans_h0; i <= trans_h1 && i < device->height; i++) {
					for (j = trans_w0; j <= trans_w1 && j < device->width; j++) {
						index = i * device->width + j;
						__trans = &device->_trans[index];
						if (*__trans != EP_BLACK) {
							__tango = &device->_tango[index];
							*__tango = *__trans;
							*__trans = EP_BLACK;
						}
					}
				}
			}

	}


	void Render(Manager3D& man, VObj * range, VObj * range0, VObj * range1) {
		VObj * _range;

		Mat3D mm;

		Obj3D * obj = man.objs.link;
		if (obj) {
			INT render_state = 0;
			INT trans_w0 = EP_MAX, trans_h0 = EP_MAX;
			INT trans_w1 = -EP_MAX, trans_h1 = -EP_MAX;
			VObj * v, *v0, *v1, *vtemp;
			EPoint l1, l0, l;

			EFTYPE z;
			INT index = 0, _index = 0;
			INT xs, xe, ys, ye;
			INT i, j;
			INT res;
			Camera3D* cam = NULL;
			Lgt3D * lgt;
			EFTYPE zz;
			EFTYPE f, t, transparent, _i, _j;
			INT line_state = 0;
			INT line_l = 0, line_r = 0;
			INT inrange;
			do {
				v = obj->verts_r.link;
				// more than 3 verts
				if (v && obj->verts_r.linkcount >= 3) {
					v0 = NULL; v1 = NULL;
					INT traverseCount = 0;
					do {
						traverseCount++;
						//there must be three verts
						if (v0 && v1) {
							// back face culling
							if (v->backface > 0) 
							{

								_range = NULL;
								if (range) {
									_range = range;
								}
								else {
									_range = v;
								}
								// in range
								inrange = true;
								if (_range != v) {
									inrange = EPoint::RectIsIntersect(v->xs, v->ys, v->xe, v->ye, _range->xs, _range->ys, _range->xe, _range->ye);
									//inrange = Vert3D::CrossRect(v->xs, v->ys, v->xe, v->ye, _range->xs, _range->ys, _range->xe, _range->ye);
								}
								if (_range && inrange) {

									//step1: render the triangle
									index = 0;
									xs = v->xs; xe = v->xe; ys = v->ys; ye = v->ye;
									//xs = _range == v ? v->xs : max(_range->xs, v->xs); ys = _range == v ? v->ys : max(_range->ys, v->ys);
									//xe = _range == v ? v->xe : min(_range->xe, v->xe); ye = _range == v ? v->ye : min(_range->ye, v->ye);
									//draw triangle contour
									Draw_Line(_image, width, height, v0->x0, v0->y0, v1->x0, v1->y0, WHITE);
									Draw_Line(_image, width, height, v1->x0, v1->y0, v->x0, v->y0, WHITE);
									Draw_Line(_image, width, height, v->x0, v->y0, v0->x0, v0->y0, WHITE);

									//get line formula
									//v0-v1
									Vert3D::GetLine(v1->v_s, v0->v_s, l1);
									//v1-v
									Vert3D::GetLine(v->v_s, v1->v_s, l);
									//v-v0
									Vert3D::GetLine(v0->v_s, v->v_s, l0);

									EFTYPE zz_f = (v->n_r.x * v->v_c.x + v->n_r.y * v->v_c.y + v->n_r.z * v->v_c.z);
									for (i = ys; i <= ye && i < height; i += 1) {
										cam = obj->cam;
										if (cam == NULL) {
											break;
										}
										//little trick^_^
										line_state = 0;
										line_l = 0, line_r = 0;
										if (false && render_linear < 0) {
											line_l = xs;
											line_r = xe;
										}
										else {
											//trick: pre-judge
											for (j = xs; j <= xe && j < width; j += 1) {
												__image = &_image[i * width + j];
												//up pulse
												if (*__image != EP_BLACK) {
													line_state++;
													if (line_state == 1) {
														line_l = j;
													}
													else {//if (line_state == 2) {
														line_r = j;
													}
													*__image = EP_BLACK;
												}
											}
										}
										//get range x
										EFTYPE __y = i;
										EFTYPE __x;
										INT _line_l1 = (INT)(l1.x * __y + l1.y);
										INT _line_l = (INT)(l.x * __y + l.y);
										INT _line_l0 = (INT)(l0.x * __y + l0.y);
										EFTYPE view_h = (i - cam->offset_h) / cam->scale_h;
										for (j = line_l; j <= line_r  && j < width; j += 1) {
											index = i * width + j;
											__image = &_image[index];
											if (render_linear < 0) {
												if (j == line_l || j == line_r) {
													*__image = obj->color;
												}
											}
											else {
												if (j >= line_l && j <= line_r) {
													*__image = obj->color;
												}
											}
											//step2: depth test
											if (*__image != EP_BLACK) {
												__depth = &_depth[index];

												// get depth
												//(-n.x * ((EFTYPE)j - v.x) - n.y * ((EFTYPE)i - v.y)) / n.z + v->z
												n0.set((j - cam->offset_w) / cam->scale_w, view_h, 0, 1);
												//z = Vert3D::getZ(v->n_d, v->x0, v->y0, v->z0, (EFTYPE)j, (EFTYPE)i);
												z = Vert3D::getZ(v->n_1_z, v->x, v->y, v->z, n0.x, n0.y);
												if (EP_ISZERO(*__depth)) {
													//*__depth = z;
												}
												if (*__depth < z) {
													*__depth = z;

													__tango = &_tango[index];
													__trans = &_trans[index];

													//step3: render light
													//n0.set((j - cam->offset_w) / cam->scale_w, (i - cam->offset_h) / cam->scale_h, z, 1);
													n0.z = z;
													// get position
													n0 * cam->proj_1;
													zz = zz_f / (v->n_r.x * n0.x + v->n_r.y * n0.y + v->n_r.z);
													n0.x *= zz;
													n0.y *= zz;
													n0.z = zz;
													n0.w = 1;
													n1.set(n0)* cam->M_1;

													*__image = obj->getTextureColor(obj, n0, n1, n2, n3, v, NULL);

													//get interpolation normal vector from 3 points of a triangle
													__x = j;
													if (1 == obj->normal_type) {
														Object3D_GetInterpolationNormalVector(v0, v1, v, __x, __y,
															line_r, line_l, _line_l1, _line_l, _line_l0,
															5, _n0, _n1, _n2, _n3);
													}
													else {
														_n0.set(v->n_r);
													}

													//calculate sumption of light factors
													lgt = man.lgts.link;
													f = 0;
													if (lgt) {
														do {
															f += lgt->getFactor(lgt, _n0, n0);

															if (render_light < 0) {
																break;
															}

															lgt = man.lgts.next(&man.lgts, lgt);
														} while (lgt && lgt != man.lgts.link);
													}

													//step4: render transparent
													if (!EP_ISZERO(obj->transparent)) {
														r.set(n0);
														//t = r.negative() & v->n_r;
														t = r.negative() & _n0;

														if (t < 0) t = -t;
														transparent = 1.0 / obj->transparent;
														if (transparent < 0) transparent = -transparent;

														_i = (i - obj->center_r.y) * (transparent / t) + obj->center_r.y;
														_j = (j - obj->center_r.x) * (transparent / t) + obj->center_r.x;

														if (obj->transparent < 0) {
															_i = 2 * obj->center_r.y - _i;
															_j = 2 * obj->center_r.x - _j;
														}
														if (!(_i < 0 || _i > height - 1 || _j < 0 || _j > width - 1)) {
															_index = (INT)_i * width + (INT)_j;
															//if (depth[_index] < z) 

															//*__trans = Light3D_multi(*__image, f);
															*__trans = Light3D_add(*__image, _tango[_index], f);

															if (*__trans == EP_BLACK) {
																//*__trans++;
																*__trans = *__image;
															}
															//get transparent range
															if (trans_w1 < j) trans_w1 = j;
															if (trans_h1 < i) trans_h1 = i;
															if (trans_w0 > j) trans_w0 = j;
															if (trans_h0 > i) trans_h0 = i;
														}
													}
													else {
														*__image = Light3D_multi(*__image, f);
													}
													*__tango = *__image;

													//step5: render shadow map
													lgt = man.lgts.link;
													INT shadeIndex = 0;
													EFTYPE * _shade = NULL;

													if (lgt) {
														do {
															if (shadeIndex >= light_count) {
																break;
															}
															_shade = shade[shadeIndex++];

															n2.set(n1) * lgt->M_1;
															cam->project(cam, n2);
															_j = (int)(n2.x * cam->scale_w + cam->offset_w), _i = (int)(n2.y * cam->scale_h + cam->offset_h);

															if (!(_i < 0 || _i > height - 1 || _j < 0 || _j > width - 1)) {
																_index = _i * width + _j;

																__shade = &_shade[_index];
																if (render_proj > 0) {
																	_tango[_index] = RED;// obj->color;
																}

																//shadow
																if (EP_GTZERO(*__shade - n2.z - 1e-1)) {
																	*__tango = Light3D_multi(*__tango, f / 5);
																}
															}

															lgt = man.lgts.next(&man.lgts, lgt);
														} while (lgt && lgt != man.lgts.link);
													}

													if (render_proj > 0) {
														n2.set(n0)*cam->M_1 *obj->M_1* obj->M* cam->M;
														//n2.set(n1) * cam->M;
														cam->project(cam, n2);

														INT __j = (int)(n2.x * cam->scale_w + cam->offset_w), __i = (int)(n2.y * cam->scale_h + cam->offset_h);
														//Draw_Line(_tango, width, height, __j, __i, _j, _i, RED);
														_j = __j, _i = __i;

														if (!(_i < 0 || _i > height - 1 || _j < 0 || _j > width - 1)) {
															_index = _i * width + _j;
															_tango[_index] = BLUE;// obj->color
														}
													}
												}

												if (render_linear < 0) {
													_image[index] = EP_BLACK;
												}
												else {
													_image[index] = EP_BLACK;
												}
											}
										}
									}
								}
							}

							if (obj->vertex_type == 1) {
								v0 = NULL;
								v1 = NULL;
							}
							else if (obj->vertex_type == 2) {
								if ((traverseCount + 1) % 2 == 0) {
									v0 = v;
									//this->v1 = this->v1;
								}
								else {
									v0 = v1;
									v1 = v;
								}
							}
							else {
								v0 = v1;
								v1 = v;
							}
						}
						else if (v0 == NULL) {
							v0 = v;
						}
						else if (v1 == NULL) {
							v1 = v;
						}

						v = obj->verts_r.next(&obj->verts_r, v);
					} while (v && v != obj->verts_r.link);
				}

				//first do objects till end
				//then do reflection and then transparent object
				if (render_state == 0) {
					obj = man.objs.next(&man.objs, obj);
					if (!(obj && obj != man.objs.link)) {
						//obj = man.tras.link;
						////do not render reflection points
						//render_state = 2;
						obj = man.refl.link;
						render_state = 1;
						if (!obj) {
							obj = man.tras.link;
							render_state = 2;
						}
					}
				}
				else if (render_state == 1) {
					obj = man.refl.next(&man.refl, obj);
					if (!(obj && obj != man.refl.link)) {
						obj = man.tras.link;
						render_state = 2;
					}

				}
				else {
					obj = man.tras.next(&man.tras, obj);
					if (!(obj && obj != man.tras.link)) {
						//render transparent after all transparent objects were done
						index = 0;
						for (i = trans_h0; i <= trans_h1 && i < height; i++) {
							for (j = trans_w0; j <= trans_w1 && j < width; j++) {
								index = i * width + j;
								__trans = &_trans[index];
								if (*__trans != EP_BLACK) {
									__tango = &_tango[index];
									*__tango = *__trans;
									*__trans = EP_BLACK;
								}
							}
						}

						break;
					}
				}
			} while (obj);
		}
	}

	
	void ClearBeforeRayTracing() {
		memset(raytracing, 0, width * height * sizeof(DWORD));
	}
	void RenderRayTracing_SingleThread(Manager3D& man) {
		extern VertsPoolImp vertsPoolImp;
		VertsPoolImp * pools = &vertsPoolImp;
		if (render_mode == 0) {
			RenderRayCastingSub(man, 0, 0, width, height, 0, NULL, this, pools);
		}
		else {
			RenderRayTracingSub(man, 0, 0, width, height, 0, NULL, this, pools);
		}
	}

	void SetRect(RenderParameters& p, INT sx, INT sy, INT ex, INT ey) {
		p.sx = sx;
		p.sy = sy;
		p.ex = ex;
		p.ey = ey;
	}
	HANDLE thread_main;
	HANDLE * thread_pool = NULL;
	HANDLE hMutex;
	RenderParameters * param = NULL;
	void * pool = NULL;
	void* vobjPool = NULL;
	void* objPool = NULL;
	void* recordsPool = NULL;
	INT thread_ready;
	INT * thread_status = NULL;
	INT thread_all_done;

	void drawThreadSplit() {
		DWORD * _tango = EP_GetImageBuffer();
		INT dx = width / (thread_count_h);
		INT dy = height / (thread_count);
		for (INT i = 0; i < height; i++) {
			for (INT j = 0; j < width; j++) {
				if (j % dx == 0 || i % dy == 0) {
					_tango[j + i * width] = RED;
				}
			}
		}
	}

	void RenderRayTracing(Manager3D& man) {
		if (0 == thread_ready) {
			//线程数 thread_count * thread_count_h
			//max is limit by MAX_OBJ3D_MAX macro
			//创建互斥体  
			hMutex = CreateMutex(NULL, FALSE, TEXT("Mutex"));
			//创建线程  
			INT dx = width / thread_count_h;
			INT dy = height / thread_count;
			if (thread_pool == NULL) {
				thread_pool = new HANDLE[thread_count_h * thread_count];
			}
			if (thread_status == NULL) {
				thread_status = new INT[thread_count_h * thread_count];
			}
			if (param == NULL) {
				param = new RenderParameters[thread_count_h * thread_count];
			}
			if (pool == NULL) {
				//pool = new VertsPoolImp[thread_count * thread_count];
				pool = malloc(sizeof(VertsPoolImp) * thread_count_h * thread_count);
			}
			if (vobjPool == NULL) {
				vobjPool = malloc(sizeof(VObjMiniPoolImp) * thread_count_h * thread_count);
			}
			if (objPool == NULL) {
				objPool = malloc(sizeof(ObjPoolImp) * thread_count_h * thread_count);
			}
			if (recordsPool == NULL) {
				recordsPool = malloc(sizeof(RecordsPoolImp) * thread_count_h * thread_count);
			}
			for (INT i = 0; i < thread_count_h; i++) {
				for (INT j = 0; j < thread_count; j++) {
					INT index = i * thread_count + j;

					param[index].man = &man;
					param[index].device = this;
					param[index].id = index;
					param[index].hMutex = hMutex;
					SetRect(param[index], dx * i, dy * j, dx * i + dx, dy * j + dy);

					param[index].pool = pool;
					param[index].objPool = objPool;
					param[index].vobjPool = vobjPool;
					param[index].recordsPool = (RecordsPoolImp*)recordsPool + i * thread_count + j;

					thread_pool[index] = CreateThread(NULL, 0, RenderThreadProc, &param[index], 0, NULL);
					param[index].hThread = thread_pool[index];
					thread_status[index] = 1;
				}
			}
			thread_ready = 1;
		}
		else {
			for (INT i = 0; i < thread_count_h; i++) {
				for (INT j = 0; j < thread_count; j++) {
					INT index = i * thread_count + j;
					param[index].man = &man;
					param[index].device = this;

					ResumeThread(thread_pool[index]);
					thread_status[index] = 1;
				}
			}
		}
		//最后启动主线程并运行
		thread_main = CreateThread(NULL, 0, RenderThreadMain, this, 0, NULL);
		//句柄不再用到就关闭
		CloseHandle(thread_main);
		//调用的线程退出以进行后续显示工作
		return;
	}
	void EndAllThread() {
		//没有运行线程
		if (0 == thread_ready) {
			return;
		}
		//没有运行线程资源
		if (!thread_pool || !param || !thread_status) {
			return;
		}
		for (INT i = 0; i < thread_count_h; i++) {
			for (INT j = 0; j < thread_count; j++) {
				INT index = i * thread_count + j;
				//将这些参数设置为NULL以结束线程
				param[index].man = NULL;

				ResumeThread(thread_pool[index]);
			}
		}
		//等待线程退出  
		for (INT i = 0; i < thread_count_h; i++) {
			for (INT j = 0; j < thread_count; j++) {
				WaitForSingleObject(thread_pool[i * thread_count + j], INFINITE);
			}
		}

		//关闭句柄，释放资源  
		for (INT i = 0; i < thread_count_h; i++) {
			for (INT j = 0; j < thread_count; j++) {
				CloseHandle(thread_pool[i * thread_count + j]);
			}
		}
		//释放互斥资源
		CloseHandle(hMutex);
		//清除资源
		if (thread_pool) {
			delete[] thread_pool;
		}
		if (thread_status) {
			delete[] thread_status;
		}
		if (param) {
			delete[] param;
		}
		if (pool) {
			delete[] pool;
		}
		if (objPool) {
			delete[] objPool;
		}
		if (vobjPool) {
			delete[] vobjPool;
		}
		if (recordsPool) {
			delete[] recordsPool;
		}
	}
	static DWORD WINAPI RenderThreadMain(LPVOID lpThreadParameter) {
		Device * device = (Device*)lpThreadParameter;
		if (NULL == device) {
			return 0;
		}
		device->thread_all_done = 1;
		//等待线程退出
		INT _thread_all_done;
		while (1) {
			_thread_all_done = 1;
			for (INT i = 0; i < device->thread_count_h; i++) {
				for (INT j = 0; j < device->thread_count; j++) {
					INT index = i * device->thread_count + j;
					if (1 == device->thread_status[index]) {
						_thread_all_done = 0;
						break;
					}
				}
				if (0 == _thread_all_done) {
					break;
				}
			}
			if (_thread_all_done) {
				break;
			}
		}
		device->thread_all_done = 0;
		return 0;
	}
	static DWORD WINAPI RenderThreadProc(LPVOID lpThreadParameter) {
		RenderParameters * pthread = (RenderParameters*)lpThreadParameter;
		if (NULL == pthread) {
			return 0;
		}
		while (1) {
			if (NULL == pthread->man) {
				break;
			}
			if (pthread->device->render_mode == 0) {
				RenderRayCastingSub(*pthread->man, pthread->sx, pthread->sy, pthread->ex, pthread->ey, pthread->id, pthread->hMutex, pthread->device, pthread->pool, pthread->objPool, pthread->vobjPool);
			}
			else {
				RenderRayTracingSub(*pthread->man, pthread->sx, pthread->sy, pthread->ex, pthread->ey, pthread->id, pthread->hMutex, pthread->device, pthread->pool, pthread->objPool, pthread->vobjPool, pthread->recordsPool);
			}
			pthread->device->thread_status[pthread->id] = 0;
			SuspendThread(pthread->hThread);
		}
		return 0;
	}

//使用同一个pool因此要保证操作的原子性
#define RAYTRACING_MUTEX(x) \
	if (hMutex) {WaitForSingleObject(hMutex, 1000);} ##x\
	if (hMutex) { ReleaseMutex(hMutex); }

	//ray casting
	static void RenderRayCastingSub(Manager3D & man, INT sx, INT sy, INT ex, INT ey, INT id, HANDLE hMutex, Device* device = NULL, void * pools = NULL, void * objPool = NULL, void * vobjPool = NULL) {
		if (NULL == device) {
			return;
		}
		if (ex < sx) return;
		if (ey < sy) return;
		Cam3D * cam = man.cams.link;
		if (NULL == cam) {
			return;
		}

		Lgt3D * lgt;
		EFTYPE f;
		Vert3D n0, n1, n2, n3, p;
		Vert3D _n0, _n1, _n2, _n3;
		EPoint l1, l0, l;
		EFTYPE z;
		Ray ray;
		INT index;
		EFTYPE _i, _j;
		INT _index;
		DWORD * _raytracing;
		EFTYPE trans;
		VertsPoolImp * pool = &((VertsPoolImp*)pools)[id];
		_VertsPoolImp(pool);
		//VertsMan raytracing_verts(pool, 0);
		//VertsMan raytracing_verts_accumulated(pool, 1);
		VertsMan raytracing_verts;
		_VertsMan(&raytracing_verts, 0, pool);
		VertsMan raytracing_verts_accumulated;
		_VertsMan(&raytracing_verts_accumulated, 1, pool);

		VObjMan * link = NULL;
		ObjMan * olink;
		//MultiLinkList<Obj3D> octs(MAX_OBJ3D_LINK + 1 + id);
		ObjMan octs;
		DWORD * __image;
		//reflection times
		INT count, shadow_count;
		//for each pixel in width * height's screen
		for (int y = sy; y < ey; y++) {
			for (int x = sx; x < ex; x++) {
				//Orthographic
				if (cam->cam_type == 1) {
					//get original vert from this pixel
					n0.set((x - cam->offset_w) / cam->scale_w, (y - cam->offset_h) / cam->scale_h, 0, 1);
					//get direction vert
					n1.set(cam->lookat).negative();
					n1.normalize().negative();
					//set ray
					ray.set(n0, n1);
					//set ray type
					ray.type = 0;
				}
				//Oblique
				else if (cam->cam_type == 2) {
					//get original vert from this pixel
					n0.set(0, 0, 0, 1);
					//get direction vert
					n2.set((x - cam->offset_w) / cam->scale_w, (y - cam->offset_h) / cam->scale_h, 0, 1);
					cam->anti_normalize(cam, n2, cam->znear);
					//n0.set(n2);
					n1.set(cam->lookat) * cam->znear;
					n1 + n2;
					n1.w = 1;
					n1.normalize().negative();
					//set ray
					ray.set(n0, n1);
					//set ray type
					ray.type = 0;
				}
				index = y * device->width + x;
				_raytracing = &device->raytracing[index];

				Verts * nearest_vert_shadow = NULL;
				Lgt3D * cur_lgt = man.lgts.link;
				shadow_count = 0;
				//ray tracing depth
				count = device->raytracing_count;
				do {
					// when the ray is reflection or refraction
					// use the objects around instead of all the objects

					if (0 && (1 == ray.type || 2 == ray.type)) {
						if (!ray.obj) {
							ray.obj = ray.obj;
						}
						olink = &octs;
						//olink = &man.octs;
						olink->clearLink(olink);
						man.octTree.CollisionA(&man.octTree, man.octTree.link, (Obj3D*)ray.obj, olink);
					}
					else {
						olink = &man.objs;
					}

					Obj3D * obj = olink->link;
					if (obj) {
						int render_state = 0;
						VObj * v, *v0, *v1, *vtemp;

						EFTYPE trans_last = 1000;
						// for each triangle
						do {
							//object aabb intersection
							INT intersection = 1;
							if (&man.objs == olink) {
								//intersection = man.octTree.Collision(&man.octTree, man.octTree.link, ray.original, ray.direction, (Camera3D*)cam, obj);
								intersection = Collision(ray.original, ray.direction, obj, trans_last);
							}
							if (intersection) {

								// when the ray is reflection
								// then use all the verts instead 
								// of the verts after frustrum culling
								if (1 == ray.type) {
									link = &obj->verts;
								}
								else {
									link = &obj->verts_r;
								}
								v = link->link;
								// more than 3 verts
								if (v && link->linkcount >= 3) {
									v0 = NULL; v1 = NULL;
									int traverseCount = 0;
									do {
										traverseCount++;
										//there must be three verts
										if (v0 && v1) {
											// back face culling
											// when the ray is reflection or shadow testing
											// then do not need back face culling
											if (v->backface > 0 || 1 == ray.type || (3 == ray.type && obj->backfaceculling == 0))
											{
												//NOTE: ray tracing is in camera coordinate
												//get intersect point
												trans = Vert3D::GetLineIntersectPointWithTriangle(v->v_c, v0->v_c, v1->v_c, ray.original, ray.direction, trans_last, p);
												//trans is greater than zero, and less than last trans
												if (EP_GTZERO(trans)) {
													//RAYTRACING_MUTEX(Verts * verts = new Verts(););
													Verts * verts = pool->vertsPool.get(&pool->vertsPool);
													if (!verts) {
														verts = verts;
													}
													else {
														_Verts(verts);
														trans_last = trans;
														verts->v.set(p);
														verts->trans = trans;
														verts->n_r.set(v->n_r);
														verts->obj = obj;
														raytracing_verts.insertLink(&raytracing_verts, verts, NULL, NULL);
														__image = &verts->color;

														//shadow test set color to black or white
														//then stop ray tracing
														if (3 == ray.type) {
															*__image = Light3D_multi(ray.color, ray.f / 5);
															verts->type = 0;
															break;
														}
														else {
															n0.set(p);
															n1.set(n0)* cam->M_1;
															//get texture and normal vector at the same time
															*__image = obj->getTextureColor(obj, n0, n1, n2, n3, v, &verts->v_n);

															if (1 == obj->normal_type) {
																//get line formula
																//v0-v1
																Vert3D::GetLine(v1->v_s, v0->v_s, l1);
																//v1-v
																Vert3D::GetLine(v->v_s, v1->v_s, l);
																//v-v0
																Vert3D::GetLine(v0->v_s, v->v_s, l0);
																//get range x
																_n1.set(n0);
																cam->normalize(cam, _n1);
																_n2.set(_n1.x * cam->scale_w + cam->offset_w, _n1.y * cam->scale_h + cam->offset_h, _n1.z);
																EFTYPE __y = _n2.y;
																EFTYPE __x = _n2.x;
																INT _line_l1 = (INT)(l1.x * __y + l1.y);
																INT _line_l = (INT)(l.x * __y + l.y);
																INT _line_l0 = (INT)(l0.x * __y + l0.y);
																INT line_l, line_r;
																INT minx, maxx;
																minx = min(min(v->x0, v0->x0), min(v->x0, v1->x0));
																maxx = max(max(v->x0, v0->x0), max(v->x0, v1->x0));
																if (_line_l1 < minx || _line_l1 > maxx) {
																	_line_l1 = 0;
																	line_l = min(_line_l, _line_l0);
																	line_r = max(_line_l, _line_l0);
																}
																else if (_line_l < minx || _line_l > maxx) {
																	_line_l = 0;
																	line_l = min(_line_l1, _line_l0);
																	line_r = max(_line_l1, _line_l0);
																}
																else if (_line_l0 < minx || _line_l0 > maxx) {
																	_line_l0 = 0;
																	line_l = min(_line_l1, _line_l);
																	line_r = max(_line_l1, _line_l);
																}
																else {
																	line_l = min(min(_line_l, _line_l0), min(_line_l1, _line_l0));
																	line_r = max(max(_line_l, _line_l0), max(_line_l1, _line_l0));
																}
																//get interpolation normal vector from 3 points of a triangle
																Object3D_GetInterpolationNormalVector(v0, v1, v, __x, __y,
																	line_r, line_l, _line_l1, _line_l, _line_l0,
																	5, _n0, _n1, _n2, _n3);
																verts->v_3.set(_n0);
															}
															else {
																_n0.set(v->n_r);
																verts->v_3.set(verts->v_n);
															}

															//calculate sumption of light factors
															lgt = man.lgts.link;
															f = 0;
															if (lgt) {
																do {
																	f += lgt->getFactor(lgt, _n0, n0);

																	if (device->render_light < 0) {
																		break;
																	}

																	lgt = man.lgts.next(&man.lgts, lgt);
																} while (lgt && lgt != man.lgts.link);
															}


															//normal verts
															if (0 == render_state) {
																*__image = Light3D_multi(*__image, f);
																//set type normal
																verts->type = 0;
															}
															//reflection verts
															else if (1 == render_state) {
																*__image = Light3D_add(*__image, EP_BLACK, f / 2);
																//*__image = Light3D_multi(EP_BLACK, f);
																//set type reflection
																verts->type = 1;
															}
															//transparent verts
															else if (2 == render_state) {
																*__image = Light3D_add(*__image, EP_BLACK, f / 2);
																//*__image = Light3D_multi(EP_BLACK, f);
																//set type transparent
																verts->type = 2;
															}
														}
													}
													//when the ray is reflection,
													//there will be one or two hit point
													//in other case, because of using backface cull,
													//there will be only one hit point
													if (!(1 == ray.type)) {
														break;
													}
												}
											}

											if (obj->vertex_type == 1) {
												v0 = NULL;
												v1 = NULL;
											}
											else if (obj->vertex_type == 2) {
												if ((traverseCount + 1) % 2 == 0) {
													v0 = v;
													//this->v1 = this->v1;
												}
												else {
													v0 = v1;
													v1 = v;
												}
											}
											else {
												v0 = v1;
												v1 = v;
											}
										}
										else if (v0 == NULL) {
											v0 = v;
										}
										else if (v1 == NULL) {
											v1 = v;
										}

										v = link->next(link, v);
									} while (v && v != link->link);
								}
							}

							// use the objects around or all the objects?
							if (&man.objs == olink) {
								//first do objects till end
								//then do reflection and then transparent object
								if (render_state == 0) {
									obj = man.objs.next(&man.objs, obj);
									if (!(obj && obj != man.objs.link)) {
										obj = man.refl.link;
										//next render reflection points
										render_state = 1;
										if (!obj) {
											//or render reflection points
											obj = man.tras.link;
											render_state = 2;
										}
									}
								}
								else if (render_state == 1) {
									obj = man.refl.next(&man.refl, obj);
									if (!(obj && obj != man.refl.link)) {
										obj = man.tras.link;
										//next render transparent points
										render_state = 2;
									}

								}
								else {
									obj = man.tras.next(&man.tras, obj);
									if (!(obj && obj != man.tras.link)) {
										break;
									}
								}
							}
							else {
								obj = olink->next(olink, obj);
								if (!(obj && obj != olink->link)) {
									break;
								}
							}
						} while (obj);
					}
					//make sure all temporary links are cleaned
					if (&man.objs == olink) {
					}
					else {
						olink->clearLink(olink);
					}
					//get the nearest verts from all the ray traced verts
					Verts * verts = raytracing_verts.link;
					Verts * nearest_vert = verts;
					if (verts) {
						do {
							if (verts->trans < nearest_vert->trans) {
								nearest_vert = verts;
							}

							verts = raytracing_verts.next(&raytracing_verts, verts);
						} while (verts && verts != raytracing_verts.link);
					}
					if (nearest_vert) {
						raytracing_verts_accumulated.insertLink(&raytracing_verts_accumulated, nearest_vert, NULL, NULL);
						//RAYTRACING_MUTEX(raytracing_verts.~MultiLinkList(););
						raytracing_verts.clearLink(&raytracing_verts);

						//normal verts
						if (0 == nearest_vert->type) {
							//get shadow test ray
							cur_lgt = man.lgts.link;
							if (cur_lgt) {
								do {
									n2.set(0, 0, 0, 1) * cur_lgt->M *cam->M;
									n2 - nearest_vert->v;
									n2.normalize();
									ray.set(nearest_vert->v, n2);
									//set object
									ray.obj = nearest_vert->obj;
									//set ray type
									ray.type = 3;
									//this color
									ray.color = nearest_vert->color;
									//this factor
									ray.f = cur_lgt->getFactor(cur_lgt, nearest_vert->v_3, nearest_vert->v);

									__image = &nearest_vert->color;

									*__image = ray.color;
									Obj3D * obj = olink->link;
									if (obj) {
										int render_state = 0;
										VObj * v, *v0, *v1, *vtemp;

										EFTYPE trans_last = 1000;
										// for each triangle
										do {
											//object aabb intersection
											INT intersection = 1;
											if (&man.objs == olink) {
												//intersection = man.octTree.Collision(&man.octTree, man.octTree.link, ray.original, ray.direction, (Camera3D*)cam, obj);
												intersection = Collision(ray.original, ray.direction, obj, trans_last);
											}
											if (intersection) {

												// when the ray is reflection
												// then use all the verts instead 
												// of the verts after frustrum culling
												if (1 == ray.type) {
													link = &obj->verts;
												}
												else {
													link = &obj->verts_r;
												}
												v = link->link;
												// more than 3 verts
												if (v && link->linkcount >= 3) {
													v0 = NULL; v1 = NULL;
													int traverseCount = 0;
													do {
														traverseCount++;
														//there must be three verts
														if (v0 && v1) {
															// back face culling
															// when the ray is reflection or shadow testing
															// then do not need back face culling
															if (v->backface > 0 || 1 == ray.type || (3 == ray.type && obj->backfaceculling == 0))
															{
																//NOTE: ray tracing is in camera coordinate
																//get intersect point
																trans = Vert3D::GetLineIntersectPointWithTriangle(v->v_c, v0->v_c, v1->v_c, ray.original, ray.direction, trans_last, p);
																//trans is greater than zero, and less than last trans
																if (EP_GTZERO(trans)) {
																	trans_last = trans;
																	*__image = Light3D_multi(*__image, ray.f / 5);

																	//caustic affect on refraction
																	if (render_state == 2) {
																		n0.set(p);
																		n1.set(n0)* cam->M_1;
																		//get texture and normal vector at the same time
																		obj->getTextureColor(obj, n0, n1, n2, n3, v, NULL);

																		if (1 == obj->normal_type) {
																			//get line formula
																			//v0-v1
																			Vert3D::GetLine(v1->v_s, v0->v_s, l1);
																			//v1-v
																			Vert3D::GetLine(v->v_s, v1->v_s, l);
																			//v-v0
																			Vert3D::GetLine(v0->v_s, v->v_s, l0);
																			//get range x
																			_n1.set(n0);
																			cam->normalize(cam, _n1);
																			_n2.set(_n1.x * cam->scale_w + cam->offset_w, _n1.y * cam->scale_h + cam->offset_h, _n1.z);
																			EFTYPE __y = _n2.y;
																			EFTYPE __x = _n2.x;
																			INT _line_l1 = (INT)(l1.x * __y + l1.y);
																			INT _line_l = (INT)(l.x * __y + l.y);
																			INT _line_l0 = (INT)(l0.x * __y + l0.y);
																			INT line_l, line_r;
																			INT minx, maxx;
																			minx = min(min(v->x0, v0->x0), min(v->x0, v1->x0));
																			maxx = max(max(v->x0, v0->x0), max(v->x0, v1->x0));
																			if (_line_l1 < minx || _line_l1 > maxx) {
																				_line_l1 = 0;
																				line_l = min(_line_l, _line_l0);
																				line_r = max(_line_l, _line_l0);
																			}
																			else if (_line_l < minx || _line_l > maxx) {
																				_line_l = 0;
																				line_l = min(_line_l1, _line_l0);
																				line_r = max(_line_l1, _line_l0);
																			}
																			else if (_line_l0 < minx || _line_l0 > maxx) {
																				_line_l0 = 0;
																				line_l = min(_line_l1, _line_l);
																				line_r = max(_line_l1, _line_l);
																			}
																			else {
																				line_l = min(min(_line_l, _line_l0), min(_line_l1, _line_l0));
																				line_r = max(max(_line_l, _line_l0), max(_line_l1, _line_l0));
																			}
																			//get interpolation normal vector from 3 points of a triangle
																			Object3D_GetInterpolationNormalVector(v0, v1, v, __x, __y,
																				line_r, line_l, _line_l1, _line_l, _line_l0,
																				5, _n0, _n1, _n2, _n3);
																		}
																		else {
																			_n0.set(v->n_r);
																		}
																		//get refraction ray
																		// refraction vector
																		//T = ((nL / nT) * N * L - SQRT(1 - (nL^2 / nT ^2)*[1 - (N * L)^2])) * N - (nL / nT) * L
																		//get n3 = N
																		//n3.set(nearest_vert->v_n);
																		n3.set(_n0);// .negative();
																		//get n2 = L
																		//this formula used a negative I
																		n2.set(ray.direction);// .negative();
																		//get n3 = T
																		EFTYPE cross = n2 ^ n3;
																		//sin(oL) <= nT / nL, that is nT > nL
																		EFTYPE nL = 0.1, nT = 0.5;
																		EFTYPE nL_nT = nL / nT;
																		EFTYPE pN = nL_nT * cross - sqrt(1 - nL_nT * nL_nT * (1 - cross * cross));
																		n3 * pN;
																		n2 * nL_nT;
																		n3 - n2;
																		n3.normalize();

																		EFTYPE f = cur_lgt->getFactor(cur_lgt, n3, p);
																		*__image = Light3D_multi(*__image, f * 5);
																	}
																	break;
																}
															}

															if (obj->vertex_type == 1) {
																v0 = NULL;
																v1 = NULL;
															}
															else if (obj->vertex_type == 2) {
																if ((traverseCount + 1) % 2 == 0) {
																	v0 = v;
																	//this->v1 = this->v1;
																}
																else {
																	v0 = v1;
																	v1 = v;
																}
															}
															else {
																v0 = v1;
																v1 = v;
															}
														}
														else if (v0 == NULL) {
															v0 = v;
														}
														else if (v1 == NULL) {
															v1 = v;
														}

														v = link->next(link, v);
													} while (v && v != link->link);
												}
											}

											// use the objects around or all the objects?
											if (&man.objs == olink) {
												//first do objects till end
												//then do reflection and then transparent object
												if (render_state == 0) {
													obj = man.objs.next(&man.objs, obj);
													if (!(obj && obj != man.objs.link)) {
														obj = man.refl.link;
														//next render reflection points
														render_state = 1;
														if (!obj) {
															//or render reflection points
															obj = man.tras.link;
															render_state = 2;
														}
													}
												}
												else if (render_state == 1) {
													obj = man.refl.next(&man.refl, obj);
													if (!(obj && obj != man.refl.link)) {
														obj = man.tras.link;
														//next render transparent points
														render_state = 2;
													}

												}
												else {
													obj = man.tras.next(&man.tras, obj);
													if (!(obj && obj != man.tras.link)) {
														break;
													}
												}
											}
											else {
												obj = olink->next(olink, obj);
												if (!(obj && obj != olink->link)) {
													break;
												}
											}
										} while (obj);
									}
									if (device->render_light < 0) {
										break;
									}

									cur_lgt = man.lgts.next(&man.lgts, cur_lgt);
								} while (cur_lgt && cur_lgt != man.lgts.link);
							}
							break;
							/*
							if (cur_lgt && (shadow_count == 0 || cur_lgt != man.lgts.link)) {
								n2.set(0, 0, 0, 1) * cur_lgt->M *cam->M;
								n2 - nearest_vert->v;
								n2.normalize();
								ray.set(nearest_vert->v, n2);
								//set object
								ray.obj = nearest_vert->obj;
								//set ray type
								ray.type = 3;
								//this color
								ray.color = nearest_vert->color;
								//this factor
								ray.f = cur_lgt->getFactor(nearest_vert->v_3, nearest_vert->v);
								//test same direction
								//EFTYPE cross = n2 & nearest_vert->v_3;
								//if (cross < 0) {
								//	//not same direction, this vertex is in shadow
								//	nearest_vert->color = Light3D_multi(nearest_vert->color, ray.f / 2);
								//}

								//shadow test does not affect ray tracing times
								count++;

								//get next shadow test light
								shadow_count++;
								nearest_vert_shadow = nearest_vert;
								cur_lgt = man.lgts.next(&man.lgts, cur_lgt);
							}
							else {
								//stop ray tracing
								break;
							}
							*/
						}
						//reflection verts
						else if (1 == nearest_vert->type) {
							//get reflection ray
							// reflection vector
							// R = I -  N * ( dot(I , N)* 2 )
							//get n3 = N
							//n3.set(nearest_vert->v_n);
							n3.set(nearest_vert->v_3);
							//get n2 = I
							n2.set(ray.direction);
							//get n2 = R
							EFTYPE cross = n2 ^ n3;
							n3 * (cross * 2);
							n2 - n3;
							n2.normalize();// .negative();
							//set ray
							ray.set(nearest_vert->v, n2);
							//set object
							ray.obj = nearest_vert->obj;
							//set ray type
							ray.type = 1;
						}
						//transparent verts
						else if (2 == nearest_vert->type) {
							//get refraction ray
							// refraction vector
							//T = ((nL / nT) * N * L - SQRT(1 - (nL^2 / nT ^2)*[1 - (N * L)^2])) * N - (nL / nT) * L
							//get n3 = N
							//n3.set(nearest_vert->v_n);
							n3.set(nearest_vert->v_3);
							//get n2 = L
							//this formula used a negative I
							n2.set(ray.direction).negative();
							//get n3 = T
							EFTYPE cross = n2 ^ n3;
							//sin(oL) <= nT / nL, that is nT > nL
							EFTYPE nL = 0.1, nT = 0.5;
							EFTYPE nL_nT = nL / nT;
							EFTYPE pN = nL_nT * cross - sqrt(1 - nL_nT * nL_nT * (1 - cross * cross));
							n3 * pN;
							n2 * nL_nT;
							n3 - n2;
							n3.normalize();
							//set ray
							ray.set(nearest_vert->v, n3);
							//set object
							ray.obj = nearest_vert->obj;
							//set ray type
							ray.type = 2;
						}
					}
					else {
						break;
					}

				} while (--count > 0);
				//RAYTRACING_MUTEX(raytracing_verts.~MultiLinkList(););
				raytracing_verts.clearLink(&raytracing_verts);

				//accumulate all the ray traced verts' color
				Verts * verts = raytracing_verts_accumulated.link;
				DWORD color = EP_BLACK;
				if (verts) {
					do {
						//if (0 == verts->type) 
						{
							color = Light3D_add(color, verts->color, 0.1);
						}

						verts = raytracing_verts_accumulated.next(&raytracing_verts_accumulated, verts);
					} while (verts && verts != raytracing_verts_accumulated.link);
				}
				//RAYTRACING_MUTEX(raytracing_verts_accumulated.~MultiLinkList(););
				raytracing_verts_accumulated.clearLink(&raytracing_verts_accumulated);

				*_raytracing = color;
			}
		}
	}
	
	//ray tracing
	static void RenderRayTracingSub(Manager3D & man, INT sx, INT sy, INT ex, INT ey, INT id, HANDLE hMutex, Device* device = NULL, void * pools = NULL, void * objPool = NULL, void * vobjPool = NULL, void * recordsPool = NULL) {
		if (NULL == device) {
			return;
		}
		if (ex < sx) return;
		if (ey < sy) return;
		Cam3D * cam = man.cams.link;
		if (NULL == cam) {
			return;
		}

		Lgt3D * lgt;
		EFTYPE f;
		Vert3D n0, n1, n2, n3, p;
		Vert3D _n0, _n1, _n2, _n3;
		EPoint l1, l0, l;
		EFTYPE z;
		Ray ray;
		INT index;
		EFTYPE _i, _j;
		INT _index;
		DWORD * _raytracing;
		EFTYPE trans;
		VertsPoolImp * pool = &((VertsPoolImp*)pools)[id];
		_VertsPoolImp(pool);
		//VertsMan raytracing_verts(pool, 0);
		//VertsMan raytracing_verts_accumulated(pool, 1);
		VertsMan raytracing_verts;
		_VertsMan(&raytracing_verts, 0, pool);
		VertsMan raytracing_verts_accumulated;
		_VertsMan(&raytracing_verts_accumulated, 1, pool);

		VObjMan * link = NULL;
		ObjMan * olink;
		//MultiLinkList<Obj3D> octs(MAX_OBJ3D_LINK + 1 + id);
		ObjMan octs;
		_VObjMiniPoolImp((VObjMiniPoolImp*)vobjPool);
		_ObjPoolImp((ObjPoolImp*)objPool, (VObjPoolImp*)vobjPool);
		_ObjMan(&octs, MAX_OBJ3D_START + 1 +id, (ObjPoolImp*)objPool, (VObjPoolImp*)vobjPool);

		RecordsMan recStack;
		RecordsPoolImp* stackPool = (RecordsPoolImp*)recordsPool;
		_RecordsPoolImp(stackPool);
		_RecordsMan(&recStack, 1, stackPool);

		DWORD * __image;
		//reflection times
		INT count, shadow_count;
		//for each pixel in width * height's screen
		for (INT y = sy; y < ey; y++) {
			for (INT x = sx; x < ex; x++) {
				//Orthographic
				if (cam->cam_type == 1) {
					//get original vert from this pixel
					n0.set((x - cam->offset_w) / cam->scale_w, (y - cam->offset_h) / cam->scale_h, 0, 1);
					//get direction vert
					n1.set(cam->lookat).negative();
					n1.normalize().negative();
					//set ray
					ray.set(n0, n1);
					//set ray type
					ray.type = 0;
				}
				//Oblique
				else if (cam->cam_type == 2) {
					//get original vert from this pixel
					n0.set(0, 0, 0, 1);
					//get direction vert
					n2.set((x - cam->offset_w) / cam->scale_w, (y - cam->offset_h) / cam->scale_h, 0, 1);
					cam->anti_normalize(cam, n2, cam->znear);
					//n0.set(n2);
					n1.set(cam->lookat) * cam->znear;
					n1 + n2;
					n1.w = 1;
					n1.normalize().negative();
					//set ray
					ray.set(n0, n1);
					//set ray type
					ray.type = 0;
				}
				index = y * device->width + x;
				_raytracing = &device->raytracing[index];


				INT i = x;
				INT j = y;
				const INT image_width = device->width;
				const INT image_height = device->height;
				DWORD color;
				Vert3D cur_color;
				Vert3D pixel_color;
				/////////////////////////////////////////////////////////////////////////
				//color = device->ray_color_1(i, j, image_width, image_height);
				//device->ray_color_2(ray, cur_color);
				//device->ray_color_3(ray, cur_color);
				//device->ray_color_4(ray, cur_color);
				/*
				device->ray_color_5(ray, cur_color);
				color = device->write_color_1(cur_color);
				*/
				pixel_color.set(0, 0, 0);
				for (int s = 0; s < device->samples_per_pixel; s++) {
					auto u = (i + random_double()) / (image_width - 1);
					auto v = (j + random_double()) / (image_height - 1);


					//Orthographic
					if (cam->cam_type == 1) {
						//get original vert from this pixel
						n0.set((x + u - cam->offset_w) / cam->scale_w, (y + v- cam->offset_h) / cam->scale_h, 0, 1);
						//get direction vert
						n1.set(cam->lookat).negative();
						n1.normalize().negative();
						//set ray
						ray.set(n0, n1);
						//set ray type
						ray.type = 0;
					}
					//Oblique
					else if (cam->cam_type == 2) {
						//get original vert from this pixel
						n0.set(0, 0, 0, 1);
						//get direction vert
						n2.set((x + u - cam->offset_w) / cam->scale_w, (y + v - cam->offset_h) / cam->scale_h, 0, 1);
						cam->anti_normalize(cam, n2, cam->znear);
						//n0.set(n2);
						n1.set(cam->lookat) * cam->znear;
						n1 + n2;
						n1.w = 1;
						n1.normalize().negative();
						//set ray
						ray.set(n0, n1);
						//set ray type
						ray.type = 0;
					}

					if (device->cam_type > 0) {
						device->rtcam->get_ray(ray, u, v);
					}
					//device->ray_color(ray, cur_color, device->max_depth, &octs);
					device->ray_color_loop(ray, cur_color, device->max_depth, &octs, recStack);
					pixel_color + cur_color;
				}
				color = device->write_color(pixel_color, device->samples_per_pixel);
				/////////////////////////////////////////////////////////////////////////


				*_raytracing = color;
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	hittable_list * world;
	SphereMan * sphereMan;
	LambertianMan * lambertianMan;
	MetalMan * metalMan;
	DielectricMan * dielectricMan;
	BvhNodeMan * bvhNodeMan;
	Manager3D* man;
	camera * rtcam;
	int samples_per_pixel = 10;
	int max_depth = 5;
	int type = 0;
	int cam_type = 0;
	int bvh = 1;
	double aspect_ratio = 1.5;
	double aperture = 2.0;
	void init_hittable() {
		if (cam_type == 2) {
			Vert3D v0, v1, v2;
			v0.set(-2, 2, 1);
			v1.set(0, 0, -1);
			v2.set(v0) - v1;
			auto dist_to_focus = sqrt(v2 ^ v2);
			v2.set(0, 1, 0);
			rtcam->set(v0, v1, v2, 90, aspect_ratio);
		}
		else if (cam_type == 3) {
			Vert3D v0, v1, v2;
			v0.set(-2, 2, 1);
			v1.set(0, 0, -1);
			v2.set(v0) - v1;
			auto dist_to_focus = sqrt(v2 ^ v2);
			v2.set(0, 1, 0);
			rtcam->set(v0, v1, v2, 20, aspect_ratio);
		}
		else if (cam_type == 4) {
			Vert3D v0, v1, v2;
			v0.set(3, 3, 2);
			v1.set(0, 0, -1);
			v2.set(v0) - v1;
			auto dist_to_focus = sqrt(v2 ^ v2);
			v2.set(0, 1, 0);
			rtcam->set(v0, v1, v2, 20, aspect_ratio, aperture, dist_to_focus);
		}
		else if (cam_type == 5) {
			Vert3D v0, v1, v2;
			v0.set(13, 2, 3);
			v1.set(0, 0, 0);
			v2.set(v0) - v1;
			auto dist_to_focus = 10.0;
			v2.set(0, 1, 0);
			aperture = 0.1;
			rtcam->set(v0, v1, v2, 20, aspect_ratio, aperture, dist_to_focus);
		}
		else if (cam_type == 6) {
			Vert3D v0, v1, v2;
			v0.set(13, 2, 3);
			v1.set(0, 0, 0);
			v2.set(v0) - v1;
			auto dist_to_focus = 10.0;
			v2.set(0, 1, 0);
			aperture = 0.1;
			rtcam->set(v0, v1, v2, 90, aspect_ratio, aperture, dist_to_focus);
		}
		if (type == 1) {
			Sphere * sphere;

			sphere = sphereMan->spherePool->get(sphereMan->spherePool);
			sphere->obj.set(0, -100.5, -1, 100);
			world->add(&sphere->obj);

			sphere = sphereMan->spherePool->get(sphereMan->spherePool);
			sphere->obj.set(0, 0, -1, 0.5);
			world->add(&sphere->obj);
		}
		else if (type == 2) {
			Lambertian * material_ground = lambertianMan->lambertianPool->get(lambertianMan->lambertianPool);
			material_ground->obj.set(0.8, 0.8, 0);
			Dielectric * material_center = dielectricMan->dielectricPool->get(dielectricMan->dielectricPool);
			material_center->obj.set(1.5);
			Metal * material_left = metalMan->metalPool->get(metalMan->metalPool);
			material_left->obj.set(0.1, 0.2, 0.5, 0.3);
			Metal * material_right = metalMan->metalPool->get(metalMan->metalPool);
			material_right->obj.set(0.8, 0.6, 0.2, 0.1);

			Sphere * sphere;
			sphere = sphereMan->spherePool->get(sphereMan->spherePool);
			sphere->obj.set(0, -100.5, -1, 100);
			sphere->obj.set_material(&material_ground->obj);
			world->add(&sphere->obj);

			sphere = sphereMan->spherePool->get(sphereMan->spherePool);
			sphere->obj.set(0, 0, -1, 0.5);
			sphere->obj.set_material(&material_left->obj);
			world->add(&sphere->obj);

			sphere = sphereMan->spherePool->get(sphereMan->spherePool);
			sphere->obj.set(-1, 0, -1, 0.5);
			sphere->obj.set_material(&material_center->obj);
			world->add(&sphere->obj);

			sphere = sphereMan->spherePool->get(sphereMan->spherePool);
			sphere->obj.set(1, 0, -1, 0.5);
			sphere->obj.set_material(&material_right->obj);
			world->add(&sphere->obj);

		}
		else if (type == 3) {
			Lambertian * ground_material = lambertianMan->lambertianPool->get(lambertianMan->lambertianPool);
			ground_material->obj.set(0.5, 0.5, 0.5);

			Sphere * sphere;
			sphere = sphereMan->spherePool->get(sphereMan->spherePool);
			sphere->obj.set(0, -1000, 0, 1000);
			sphere->obj.set_material(&ground_material->obj);
			world->add(&sphere->obj);

			Vert3D center, v0, v1;
			Vert3D albedo;
			for (int a = -11; a < 11; a++) {
				for (int b = -11; b < 11; b++) {
					auto choose_mat = random_double();
					center.set(a + 0.9*random_double(), 0.2, b + 0.9*random_double());
					v1.set(4, 0.2, 0);
					v0.set(center) - v1;

					if (sqrt(v0 ^ v0) > 0.9) {
						if (choose_mat < 0.8) {
							// diffuse
							albedo.set(random_double(), random_double(), random_double());

							Lambertian * sphere_material = lambertianMan->lambertianPool->get(lambertianMan->lambertianPool);
							sphere_material->obj.set(albedo);

							sphere = sphereMan->spherePool->get(sphereMan->spherePool);
							sphere->obj.set(center, 0.2);
							sphere->obj.set_material(&sphere_material->obj);
							world->add(&sphere->obj);
						}
						else if (choose_mat < 0.95) {
							// metal
							albedo.set(random_double(0.5, 1), random_double(0.5, 1), random_double(0.5, 1));
							auto fuzz = random_double(0, 0.5);

							Metal * sphere_material = metalMan->metalPool->get(metalMan->metalPool);
							sphere_material->obj.set(albedo, 0.3);

							sphere = sphereMan->spherePool->get(sphereMan->spherePool);
							sphere->obj.set(center, 0.2);
							sphere->obj.set_material(&sphere_material->obj);
							world->add(&sphere->obj);
						}
						else {
							// glass
							Dielectric * sphere_material = dielectricMan->dielectricPool->get(dielectricMan->dielectricPool);
							sphere_material->obj.set(1.5);

							sphere = sphereMan->spherePool->get(sphereMan->spherePool);
							sphere->obj.set(center, 0.2);
							sphere->obj.set_material(&sphere_material->obj);
							world->add(&sphere->obj);
						}
					}
				}
			}

			Dielectric * material1 = dielectricMan->dielectricPool->get(dielectricMan->dielectricPool);
			material1->obj.set(1.5);

			sphere = sphereMan->spherePool->get(sphereMan->spherePool);
			sphere->obj.set(0, 1, 0, 1.0);
			sphere->obj.set_material(&material1->obj);
			world->add(&sphere->obj);

			Lambertian * material2 = lambertianMan->lambertianPool->get(lambertianMan->lambertianPool);
			material2->obj.set(0.4, 0.2, 0.1);

			sphere = sphereMan->spherePool->get(sphereMan->spherePool);
			sphere->obj.set(-4, 1, 0, 1.0);
			sphere->obj.set_material(&material2->obj);
			world->add(&sphere->obj);


			Metal * material3 = metalMan->metalPool->get(metalMan->metalPool);
			material3->obj.set(0.7, 0.6, 0.5, 0.0);

			sphere = sphereMan->spherePool->get(sphereMan->spherePool);
			sphere->obj.set(4, 1, 0, 1.0);
			sphere->obj.set_material(&material3->obj);
			world->add(&sphere->obj);
		}
		else {
			Lambertian * material_labertian = lambertianMan->lambertianPool->get(lambertianMan->lambertianPool);
			material_labertian->obj.set(0.8, 0.8, 0);
			Dielectric * material_dielectric = dielectricMan->dielectricPool->get(dielectricMan->dielectricPool);
			material_dielectric->obj.set(1.5);
			Metal * material_metal = metalMan->metalPool->get(metalMan->metalPool);
			material_metal->obj.set(0.8, 0.6, 0.2, 0.0);

			//遍历物体，设置material
			Obj3D * obj = man->objs.link;
			if (obj) {
				do {
					obj->material = &material_labertian->obj;

					obj = man->objs.next(&man->objs, obj);
				} while (obj && obj != man->objs.link);
			}
			obj = man->refl.link;
			if (obj) {
				do {
					obj->material = &material_metal->obj;

					obj = man->refl.next(&man->refl, obj);
				} while (obj && obj != man->refl.link);
			}
			obj = man->tras.link;
			if (obj) {
				do {
					obj->material = &material_dielectric->obj;

					obj = man->tras.next(&man->tras, obj);
				} while (obj && obj != man->tras.link);
			}
		}
		if (type > 0) {
			if (bvh > 0) {
				BvhNode *bvhNode = bvhNodeMan->bvh_nodePool->get(bvhNodeMan->bvh_nodePool);
				_BvhNode(bvhNode, NULL);
				set_bvh(&bvhNode->obj, bvhNodeMan, *world, 0.0, 1.0);
				world->objects.clearLink(&world->objects);
				Hittable * hittable = world->objects.hittablePool->get(world->objects.hittablePool);
				hittable->obj = &bvhNode->obj;
				world->objects.insertLink(&world->objects, hittable, NULL, NULL);
			}
		}
	}
	bool test_hit(Ray& ray, double t_min, double t_max, hit_record& rec, ObjMan* octs) {
		rec.use_color = FALSE;
		if (type > 0) {
			return world->hit(ray, 0.001, INT_MAX, rec);
		}
		bool hit_anything = FALSE;

		Cam3D * cam = man->cams.link;
		if (NULL == cam) {
			return hit_anything;
		}
		Lgt3D * lgt;
		EFTYPE f;
		Vert3D n0, n1, n2, n3, p;
		Vert3D _n0, _n1, _n2, _n3;
		EPoint l1, l0, l;
		EFTYPE trans;

		VObjMan * link = NULL;
		ObjMan * olink;

		struct Param {
			Obj3D* obj;
			EFTYPE t;
			VObj * v, *v0, *v1;
			INT render_state;
		} param;

		Verts _verts;
		Verts * verts = &_verts;
		// when the ray is reflection or refraction
		// use the objects around instead of all the objects

		if (0 && (1 == ray.type || 2 == ray.type)) {
			if (!ray.obj) {
				ray.obj = ray.obj;
			}
			olink = octs;
			//olink = &man->octs;
			olink->clearLink(olink);
			man->octTree.CollisionA(&man->octTree, man->octTree.link, (Obj3D*)ray.obj, olink);
		}
		else {
			olink = &man->objs;
		}

		Obj3D * obj = olink->link;
		if (obj) {
			int render_state = 0;
			VObj * v, *v0, *v1, *vtemp;

			EFTYPE trans_last = 1000;
			// for each triangle
			do {
				//object aabb intersection
				INT intersection = 1;
				if (&man->objs == olink) {
					//intersection = man->octTree.Collision(&man->octTree, man->octTree.link, ray.original, ray.direction, (Camera3D*)cam, obj);
					intersection = Collision(ray.original, ray.direction, obj, trans_last);
				}
				if (intersection) {

					// when the ray is reflection
					// then use all the verts instead 
					// of the verts after frustrum culling
					if (1 == ray.type) {
						link = &obj->verts;
					}
					else {
						link = &obj->verts_r;
					}
					v = link->link;
					// more than 3 verts
					if (v && link->linkcount >= 3) {
						v0 = NULL; v1 = NULL;
						int traverseCount = 0;
						do {
							traverseCount++;
							//there must be three verts
							if (v0 && v1) {
								// back face culling
								// when the ray is reflection or shadow testing
								// then do not need back face culling
								if (v->backface > 0 || 1 == ray.type || (3 == ray.type && obj->backfaceculling == 0))
								{
									//NOTE: ray tracing is in camera coordinate
									//get intersect point
									trans = Vert3D::GetLineIntersectPointWithTriangle(v->v_c, v0->v_c, v1->v_c, ray.original, ray.direction, trans_last, p);
									//trans is greater than zero, and less than last trans
									if (EP_GTZERO(trans)) {
										//RAYTRACING_MUTEX(Verts * verts = new Verts(););
										if (verts) {
											trans_last = trans;

											param.obj = obj;
											param.v = v;
											param.v0 = v0;
											param.v1 = v1;
											param.t = trans;
											param.render_state = render_state;
											hit_anything = TRUE;

											break;
										}
										//when the ray is reflection,
										//there will be one or two hit point
										//in other case, because of using backface cull,
										//there will be only one hit point
										if (!(1 == ray.type)) {
											break;
										}
									}
								}

								if (obj->vertex_type == 1) {
									v0 = NULL;
									v1 = NULL;
								}
								else if (obj->vertex_type == 2) {
									if ((traverseCount + 1) % 2 == 0) {
										v0 = v;
										//this->v1 = this->v1;
									}
									else {
										v0 = v1;
										v1 = v;
									}
								}
								else {
									v0 = v1;
									v1 = v;
								}
							}
							else if (v0 == NULL) {
								v0 = v;
							}
							else if (v1 == NULL) {
								v1 = v;
							}

							v = link->next(link, v);
						} while (v && v != link->link);
					}
				}

				// use the objects around or all the objects?
				if (&man->objs == olink) {
					//first do objects till end
					//then do reflection and then transparent object
					if (render_state == 0) {
						obj = man->objs.next(&man->objs, obj);
						if (!(obj && obj != man->objs.link)) {
							obj = man->refl.link;
							//next render reflection points
							render_state = 1;
							if (!obj) {
								//or render reflection points
								obj = man->tras.link;
								render_state = 2;
							}
						}
					}
					else if (render_state == 1) {
						obj = man->refl.next(&man->refl, obj);
						if (!(obj && obj != man->refl.link)) {
							obj = man->tras.link;
							//next render transparent points
							render_state = 2;
						}

					}
					else {
						obj = man->tras.next(&man->tras, obj);
						if (!(obj && obj != man->tras.link)) {
							break;
						}
					}
				}
				else {
					obj = olink->next(olink, obj);
					if (!(obj && obj != olink->link)) {
						break;
					}
				}
			} while (obj);
		}
		//make sure all temporary links are cleaned
		if (&man->objs == olink) {
		}
		else {
			olink->clearLink(olink);
		}
		if (hit_anything) {

			Obj3D* obj = param.obj;
			INT render_state = param.render_state;
			VObj *v, *v0, *v1;
			v = param.v;
			v0 = param.v0;
			v1 = param.v1;

			_Verts(verts);

			verts->v.set(p);
			verts->trans = trans;
			verts->n_r.set(v->n_r);
			verts->obj = obj;
			__image = &verts->color;

			//shadow test set color to black or white
			//then stop ray tracing
			if (3 == ray.type) {
				*__image = Light3D_multi(ray.color, ray.f / 5);
				verts->type = 0;
			}
			else {
				n0.set(p);
				n1.set(n0)* cam->M_1;
				//get texture and normal vector at the same time
				*__image = obj->getTextureColor(obj, n0, n1, n2, n3, v, &verts->v_n);

				if (1 == obj->normal_type) {
					//get line formula
					//v0-v1
					Vert3D::GetLine(v1->v_s, v0->v_s, l1);
					//v1-v
					Vert3D::GetLine(v->v_s, v1->v_s, l);
					//v-v0
					Vert3D::GetLine(v0->v_s, v->v_s, l0);
					//get range x
					_n1.set(n0);
					cam->normalize(cam, _n1);
					_n2.set(_n1.x * cam->scale_w + cam->offset_w, _n1.y * cam->scale_h + cam->offset_h, _n1.z);
					EFTYPE __y = _n2.y;
					EFTYPE __x = _n2.x;
					INT _line_l1 = (INT)(l1.x * __y + l1.y);
					INT _line_l = (INT)(l.x * __y + l.y);
					INT _line_l0 = (INT)(l0.x * __y + l0.y);
					INT line_l, line_r;
					INT minx, maxx;
					minx = min(min(v->x0, v0->x0), min(v->x0, v1->x0));
					maxx = max(max(v->x0, v0->x0), max(v->x0, v1->x0));
					if (_line_l1 < minx || _line_l1 > maxx) {
						_line_l1 = 0;
						line_l = min(_line_l, _line_l0);
						line_r = max(_line_l, _line_l0);
					}
					else if (_line_l < minx || _line_l > maxx) {
						_line_l = 0;
						line_l = min(_line_l1, _line_l0);
						line_r = max(_line_l1, _line_l0);
					}
					else if (_line_l0 < minx || _line_l0 > maxx) {
						_line_l0 = 0;
						line_l = min(_line_l1, _line_l);
						line_r = max(_line_l1, _line_l);
					}
					else {
						line_l = min(min(_line_l, _line_l0), min(_line_l1, _line_l0));
						line_r = max(max(_line_l, _line_l0), max(_line_l1, _line_l0));
					}
					//get interpolation normal vector from 3 points of a triangle
					Object3D_GetInterpolationNormalVector(v0, v1, v, __x, __y,
						line_r, line_l, _line_l1, _line_l, _line_l0,
						5, _n0, _n1, _n2, _n3);
					verts->v_3.set(_n0);
				}
				else {
					_n0.set(v->n_r);
					verts->v_3.set(verts->v_n);
				}

				//calculate sumption of light factors
				lgt = man->lgts.link;
				f = 0;
				if (lgt) {
					do {
						f += lgt->getFactor(lgt, _n0, n0);

						if (render_light < 0) {
							break;
						}

						lgt = man->lgts.next(&man->lgts, lgt);
					} while (lgt && lgt != man->lgts.link);
				}


				//normal verts
				if (0 == render_state) {
					*__image = Light3D_multi(*__image, f);
					//set type normal
					verts->type = 0;
				}
				//reflection verts
				else if (1 == render_state) {
					*__image = Light3D_add(*__image, EP_BLACK, f / 2);
					//*__image = Light3D_multi(EP_BLACK, f);
					//set type reflection
					verts->type = 1;
				}
				//transparent verts
				else if (2 == render_state) {
					*__image = Light3D_add(*__image, EP_BLACK, f / 2);
					//*__image = Light3D_multi(EP_BLACK, f);
					//set type transparent
					verts->type = 2;
				}
			}

			//set object
			ray.obj = obj;
			//set rec
			rec.t = verts->trans;
			rec.p.set(verts->v);
			rec.normal.set(verts->v_3).normalize();
			rec.front_face = TRUE;
			rec.use_color = TRUE;
			rec.color = verts->color;
			rec.material = (material*)obj->material;
		}
		return hit_anything;
	}
	void ray_color_loop(Ray& r, Vert3D& cur_color, int mdp, ObjMan* octs, RecordsMan& recStack) {
		Vert3D unit_direction, v0, v1;
		EFTYPE t;
		Ray scattered;

		//clear stack, do not needed if stack balance is ensured by popList
		//recStack.clearLink(&recStack);
		//push first ray in stack
		Records * records = recStack.recordsPool->get(recStack.recordsPool);
		if (!records) {
			printf("Insufficient records in pool\n");
			cur_color.set(0, 0, 0);
			return;
		}
		_Records(records, NULL);
		//set ray
		records->obj.ray.set(r.original, r.direction);
		//push
		recStack.insertLink(&recStack, records, NULL, NULL);
		//pointers
		Ray * ray = &records->obj.ray;
		Vert3D * color = &records->obj.color;
		hit_record * rec = &records->obj.rec;
		for (int i = 0; i < mdp; i++) {
			//is hit
			if (test_hit(*ray, 0.001, INT_MAX, *rec, octs)) {
				//if hitted, scatter another ray
				if (rec->material->scatter(*ray, *rec, *color, scattered)) {
					//push new ray in stack
					records = recStack.recordsPool->get(recStack.recordsPool);
					if (!records) {
						printf("Insufficient records in pool\n");
						recStack.clearLink(&recStack);
						color->set(0, 0, 0);
						break;
					}
					_Records(records, NULL);
					//set ray
					records->obj.ray.set(scattered.original, scattered.direction);
					//push
					recStack.insertLink(&recStack, records, NULL, NULL);
					//pointers
					ray = &records->obj.ray;
					color = &records->obj.color;
					rec = &records->obj.rec;
					continue;
				}
				//if the material decides not to scatter
				color->set(0, 0, 0);
			}
			else {
				//if not hit, set the color of sky
				unit_direction.set(ray->direction).normalize();
				t = 0.5*(unit_direction.y + 1.0);
				v0.set(1.0, 1.0, 1.0) * (1.0 - t);
				v1.set(0.5, 0.7, 1.0) * t;

				color->set(v0) + v1;
			}
			//accumulate all the stack colors and finish
			records = recStack.prev(&recStack, recStack.link);
			Records* last = records;
			if (records) {
				do {
					if (records) {
						long long  delta;
						delta = (long long)records - (long long)recStack.recordsPool->pool;
						if (delta % sizeof(Records) != 0) {
							delta = 10;
							printf("Error records: %lld  ", delta);
						}
						records->obj.color % (*color);
						color = &records->obj.color;
						//pointers
						ray = &records->obj.ray;
						color = &records->obj.color;
						rec = &records->obj.rec;
					}

					records = recStack.prev(&recStack, records);
				} while (records && records != last);
			}
			//do {
			//	records = recStack.popLink(&recStack);
			//	if (records) {
			//		records->obj.color % (*color);
			//		color = &records->obj.color;
			//		//pointers
			//		ray = &records->obj.ray;
			//		color = &records->obj.color;
			//		rec = &records->obj.rec;
			//	}
			//} while (records);
			break;
		}
		if (color) {
			cur_color.set(*color);
		}
		//clear stack, do not needed if stack balance is ensured by popList
		recStack.clearLink(&recStack);
	}
	void ray_color(Ray& r, Vert3D& cur_color, int mdp, ObjMan* octs) {
		if (mdp <= 0) {
			cur_color.set(0, 0, 0);
			return;
		}
		Vert3D unit_direction, v0, v1;
		EFTYPE t;
		hit_record rec;
		if (test_hit(r, 0.001, INT_MAX, rec, octs)) {
			Ray scattered;
			if (rec.material->scatter(r, rec, cur_color, scattered)) {
				ray_color(scattered, v0, mdp - 1, octs);
				cur_color % v0;
				return;
			}
			cur_color.set(0, 0, 0);
			return;
		}
		unit_direction.set(r.direction).normalize();
		t = 0.5*(unit_direction.y + 1.0);
		v0.set(1.0, 1.0, 1.0) * (1.0 - t);
		v1.set(0.5, 0.7, 1.0) * t;

		cur_color.set(v0) + v1;
	}
	void ray_color_6(Ray& r, Vert3D& cur_color, int mdp) {
		if (mdp <= 0) {
			cur_color.set(0, 0, 0);
			return;
		}
		Vert3D unit_direction, v0, v1;
		EFTYPE t;
		hit_record rec;
		if (world->hit(r, 0.001, INT_MAX, rec)) {
			//random_in_unit_sphere(v0);
			//random_unit_vector(v0);
			random_in_hemisphere(v0, rec.normal);
			v1.set(rec.p) + rec.normal + v0;
			r.set(rec.p, v1 - rec.p);
			ray_color_6(r, cur_color, mdp - 1);
			cur_color * 0.5;
			return;
		}
		unit_direction.set(r.direction).normalize();
		t = 0.5*(unit_direction.y + 1.0);
		v0.set(1.0, 1.0, 1.0) * (1.0 - t);
		v1.set(0.5, 0.7, 1.0) * t;

		cur_color.set(v0) + v1;
	}
	DWORD write_color(Vert3D& pixel_color, int samples_per_pixel) {
		auto r = pixel_color.x;
		auto g = pixel_color.y;
		auto b = pixel_color.z;

		// Divide the color by the number of samples.
		auto scale = 1.0 / samples_per_pixel;
		r = sqrt(scale * r);
		g = sqrt(scale * g);
		b = sqrt(scale * b);

		// Write the translated [0,255] value of each color component.
		return EGERGB(static_cast<int>(256 * clamp(r, 0.0, 0.999)),
			static_cast<int>(256 * clamp(g, 0.0, 0.999)),
			static_cast<int>(256 * clamp(b, 0.0, 0.999)));
	}
	void ray_color_5(Ray& r, Vert3D& cur_color) {
		Vert3D unit_direction, v0, v1;
		EFTYPE t;
		hit_record rec;
		if (world->hit(r, 0, INT_MAX, rec)) {
			v0.set(1, 1, 1);
			(cur_color.set(rec.normal) + v0) * 0.5;
			return;
		}
		unit_direction.set(r.direction).normalize();
		t = 0.5*(unit_direction.y + 1.0);
		v0.set(1.0, 1.0, 1.0) * (1.0 - t);
		v1.set(0.5, 0.7, 1.0) * t;

		cur_color.set(v0) + v1;
	}
	EFTYPE hit_sphere_3(const Vert3D& center, EFTYPE radius, const Ray& r) {
		Vert3D oc;
		oc.set(r.original) - center;
		EFTYPE a = r.direction ^ r.direction;
		EFTYPE half_b = (oc ^ r.direction);
		EFTYPE c = (oc ^ oc) - radius * radius;
		EFTYPE discriminant = half_b * half_b - a*c;
		if (discriminant < 0) {
			return -1.0;
		}
		else {
			return (-half_b - sqrt(discriminant)) / a;
		}
	}
	EFTYPE hit_sphere_2(const Vert3D& center, EFTYPE radius, const Ray& r) {
		Vert3D oc;
		oc.set(r.original) - center;
		EFTYPE a = r.direction ^ r.direction;
		EFTYPE b = 2.0 * (oc ^ r.direction);
		EFTYPE c = (oc ^ oc) - radius * radius;
		EFTYPE discriminant = b * b - 4 * a*c;
		if (discriminant < 0) {
			return -1.0;
		}
		else {
			return (-b - sqrt(discriminant)) / (2.0*a);
		}
	}
	void ray_color_4(Ray& r, Vert3D& cur_color) {
		Vert3D unit_direction, v0, v1;
		EFTYPE t;
		//t = hit_sphere_2(v0.set(0, 0, -1), 0.5, r);
		t = hit_sphere_3(v0.set(0, 0, -1), 0.5, r);
		if (t > 0) {
			r.getPoint(t, v0);
			v1.set(0, 0, -1);
			(v0 - v1).normalize();
			cur_color.set(v0.x + 1, v0.y + 1, v0.z + 1) * 0.5;
			return;
		}
		unit_direction.set(r.direction).normalize();
		t = 0.5*(unit_direction.y + 1.0);
		v0.set(1.0, 1.0, 1.0) * (1.0 - t);
		v1.set(0.5, 0.7, 1.0) * t;

		cur_color.set(v0) + v1;
	}
	bool hit_sphere_1(const Vert3D& center, EFTYPE radius, const Ray& r) {
		Vert3D oc;
		oc.set(r.original) - center;
		EFTYPE a = r.direction ^ r.direction;
		EFTYPE b = 2.0 * (oc ^ r.direction);
		EFTYPE c = (oc ^ oc) - radius * radius;
		EFTYPE discriminant = b * b - 4 * a*c;
		return (discriminant > 0);
	}
	void ray_color_3(Ray& r, Vert3D& cur_color) {
		Vert3D unit_direction, v0, v1;
		EFTYPE t;
		if (hit_sphere_1(v0.set(0, 0, -1), 0.5, r)) {
			cur_color.set(0, 0, 1);
			return;
		}
		unit_direction.set(r.direction).normalize();
		t = 0.5*(unit_direction.y + 1.0);
		v0.set(1.0, 1.0, 1.0) * (1.0 - t);
		v1.set(0.5, 0.7, 1.0) * t;

		cur_color.set(v0) + v1;
	}

	void ray_color_2(Ray& r, Vert3D& cur_color) {
		Vert3D unit_direction, v0, v1;
		unit_direction.set(r.direction).normalize();
		EFTYPE t = 0.5*(unit_direction.y + 1.0);
		v0.set(1.0, 1.0, 1.0) * (1.0 - t);
		v1.set(0.5, 0.7, 1.0) * t;

		cur_color.set(v0) + v1;
	}


	DWORD ray_color_1(INT i, INT j, INT image_width, INT image_height) {
		EFTYPE r = EFTYPE(i) / (image_width - 1);
		EFTYPE g = EFTYPE(j) / (image_height - 1);
		EFTYPE b = 0.25;

		INT ir = static_cast<int>(255.999 * r);
		INT ig = static_cast<int>(255.999 * g);
		INT ib = static_cast<int>(255.999 * b);

		return EGERGB(ir, ig, ib);
	}

	DWORD write_color_1(Vert3D& pixel_color) {
		// Write the translated [0,255] value of each color component.
		return EGERGB(static_cast<int>(255.999 * pixel_color.x),
			static_cast<int>(255.999 * pixel_color.y),
			static_cast<int>(255.999 * pixel_color.z));
	}


	/////////////////////////////////////////////////////////////////////////////////////////////


	void drawAABB(Manager3D& man, OctTree * oct) {
		if (NULL == oct) {
			return;
		}
		Cam3D * cam = man.cams.link;
		if (NULL == cam) {
			return;
		}
		if (oct->hasChild) {
			for (INT i = 0; i < MAX_QUARDANTS; i++) {
				if (oct->children[i]) {
					drawAABB(man, oct->children[i]);
				}
			}
		}
		Vert3D v[8];
		Vert3D v0, v1, v2;
		Vert3D n0, n1, n, p;
		if (render_raytracing < 0) {
			//oct tree use verts in world coordinate
			v[0].set(oct->bounds.x, oct->bounds.y, oct->bounds.z);
			v[1].set(oct->bounds.x, oct->bounds.y + oct->bounds.height, oct->bounds.z);
			v[2].set(oct->bounds.x + oct->bounds.width, oct->bounds.y + oct->bounds.height, oct->bounds.z);
			v[3].set(oct->bounds.x + oct->bounds.width, oct->bounds.y, oct->bounds.z);
			v[4].set(oct->bounds.x, oct->bounds.y, oct->bounds.z + oct->bounds.depth);
			v[5].set(oct->bounds.x, oct->bounds.y + oct->bounds.height, oct->bounds.z + oct->bounds.depth);
			v[6].set(oct->bounds.x + oct->bounds.width, oct->bounds.y + oct->bounds.height, oct->bounds.z + oct->bounds.depth);
			v[7].set(oct->bounds.x + oct->bounds.width, oct->bounds.y, oct->bounds.z + oct->bounds.depth);
			//to camera coordinate
			for (INT i = 0; i < 8; i++) {
				v[i] * cam->M;
			}
		}
		static INT indice[6][4] = {
			{ 0, 1, 2, 3},
			{ 3, 2, 6, 7},
			{ 0, 3, 7, 4},
			{ 0, 1, 5, 4},
			{ 4, 5, 6, 7},
			{ 5, 6, 2, 1}
		};
		INT intersect = 0;
		DWORD * tango = EP_GetImageBuffer();

		for (INT i = 0; i < 6; i++) {
			v0.set(v[indice[i][0]]);
			v1.set(v[indice[i][1]]);
			v2.set(v[indice[i][2]]);
			n.set(v[indice[i][3]]);

			cam->project(cam, v0);
			cam->project(cam, v1);
			cam->project(cam, v2);
			cam->project(cam, n);

			v0.x = v0.x * cam->scale_w + cam->offset_w;
			v0.y = v0.y * cam->scale_h + cam->offset_h;
			v1.x = v1.x * cam->scale_w + cam->offset_w;
			v1.y = v1.y * cam->scale_h + cam->offset_h;
			v2.x = v2.x * cam->scale_w + cam->offset_w;
			v2.y = v2.y * cam->scale_h + cam->offset_h;
			n.x = n.x * cam->scale_w + cam->offset_w;
			n.y = n.y * cam->scale_h + cam->offset_h;

			this->Draw_Line(tango, this->width, this->height, v0.x, v0.y, v1.x, v1.y, RED);
			this->Draw_Line(tango, this->width, this->height, v1.x, v1.y, v2.x, v2.y, RED);
			this->Draw_Line(tango, this->width, this->height, v2.x, v2.y, n.x, n.y, RED);
			this->Draw_Line(tango, this->width, this->height, n.x, n.y, v0.x, v0.y, RED);
		}
	}

	INT Draw_Line(DWORD* vb_start, INT lpitch, INT height,
		INT x1, INT y1, // 起始点
		INT x2, INT y2, // 终点
		DWORD color // 颜色像素
		) // video buffer and memory pitch
	{

		// this function draws a line from xo,yo to x1,y1 using differential error
		// terms (based on Bresenahams work)

		INT dx, // difference in x's
			dy, // difference in y's
			dx2, // dx,dy * 2
			dy2,
			x_inc, // amount in pixel space to move during drawing
			y_inc, // amount in pixel space to move during drawing
			error, // the discriminant i.e. error i.e. decision variable
			index; // used for looping

		// pre-compute first pixel address in video buffer
		//vb_start = vb_start + x1 + y1 * lpitch;

		// compute horizontal and vertical deltas
		dx = x2 - x1;
		dy = y2 - y1;

		// test which direction the line is going in i.e. slope angle
		if (dx >= 0)
		{
			x_inc = 1;

		} // end if line is moving right
		else
		{
			x_inc = -1;
			dx = -dx; // need absolute value

		} // end else moving left

		// test y component of slope

		if (dy >= 0)
		{
			y_inc = lpitch;
		} // end if line is moving down
		else
		{
			y_inc = -lpitch;
			dy = -dy; // need absolute value

		} // end else moving up

		// compute (dx,dy) * 2
		dx2 = dx << 1;
		dy2 = dy << 1;

		// now based on which delta is greater we can draw the line
		if (dx > dy)     //斜率小于1的情况
		{
			// initialize error term
			error = dy2 - dx;

			INT ddy = y1 * lpitch, ddx = x1;
			// draw the line
			for (index = 0; index <= dx; index++)
			{
				// set the pixel
				if (ddx >= lpitch) {
					if (ddy >= height * lpitch) {
					}
					else if (ddy <= 0) {
					}
					else {
						vb_start[lpitch - 1 + ddy] = color;
					}
				}
				else if (ddx <= 0) {
					if (ddy >= height * lpitch) {
					}
					else if (ddy <= 0) {
					} else {
						vb_start[ddy] = color;
					}
				}
				else if (ddy >= height * lpitch) {
				}
				else if (ddy <= 0) {
				}
				else {
					//*vb_start = color;
					vb_start[ddx + ddy] = color;
				}

				// test if error has overflowed
				if (error >= 0)
				{
					error -= dx2;
					// move to next line
					//vb_start += y_inc;
					ddy += y_inc;
				}
				// adjust the error term
				error += dy2;
				// move to the next pixel
				//vb_start += x_inc;
				ddx += x_inc;
			}
		}
		else   //斜率大于等于1的情况
		{
			// initialize error term
			error = dx2 - dy;

			INT ddy = y1 * lpitch, ddx = x1;
			for (index = 0; index <= dy; index++)
			{
				// set the pixel
				if (ddx >= lpitch) {
					if (ddy >= height * lpitch) {
					}
					else if (ddy <= 0) {
					}
					else {
						vb_start[lpitch - 1 + ddy] = color;
					}
				}
				else if (ddx <= 0) {
					if (ddy >= height * lpitch) {
					}
					else if (ddy <= 0) {
					}
					else {
						vb_start[ddy] = color;
					}
				}
				else if (ddy >= height * lpitch) {
				}
				else if (ddy <= 0) {
				}
				else {
					//*vb_start = color;
					vb_start[ddx + ddy] = color;
				}

				// test if error overflowed
				if (error >= 0)
				{
					error -= dy2;//这里按博主推导这里该为error+=dx2-dy2;

					// move to next line
					//vb_start += x_inc;
					ddx += x_inc;

				} // end if error overflowed

				// adjust the error term
				error += dx2;

				// move to the next pixel
				//vb_start += y_inc;
				ddy += y_inc;
			}
		}
		return(1);

	} // end Draw_Line

};



#endif//_DEVICE_H_
