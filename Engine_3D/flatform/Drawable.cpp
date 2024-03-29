
#include "Drawable.h"


Manager3D man;
Device device;
TextureManager tmand;
TextureLocalManager tman;
TextureLocalPoolImp textureLocalPoolImp;
TexturePoolImp* ptexturePoolImp;
VObjPoolImp vobjPoolImp;
ObjPoolImp objPoolImp;
CamPoolImp camPoolImp;
LgtPoolImp lgtPoolImp;
VertsPoolImp vertsPoolImp;
OctPoolImp octPoolImp;
Group3DPoolImp group3DPoolImp;

////////////////////////////////////////////////////
HittablePoolImp hittablePoolImp;
hittable_list world;
SpherePoolImp spherePoolImp;
SphereMan sphereMan;
camera rtcam;
LambertianMan lambertianMan;
LambertianPoolImp lambertianPoolImp;
MetalMan metalMan;
MetalPoolImp metalPoolImp;
DielectricMan dielectricMan;
DielectricPoolImp dielectricPoolImp;
BvhNodeMan bvhNodeMan;
BvhNodePoolImp bvhNodePoolImp;
////////////////////////////////////////////////////

struct Point3d {
	double x;
	double y;
	double z;
};
#define PI 3.141592654

Point3d getSpherePoint(double u, double v) {
	Point3d temp;
	const static double R = 1.0;
	temp.x = R * sin(PI * v) * cos(2 * PI * u);
	temp.z = R * sin(PI * v) * sin(2 * PI * u);
	temp.y = R * cos(PI * v);

	return temp;
}
////////////////////////////////////////////////////
int DEBUG_MODE = DEBUG_GRADE_2;

INT isresize = -1;
INT isrefresh = -1;
INT width;
INT height;

INT isInputBlocked() {
	//光线追踪线程运行时
	//屏蔽所有按键，防止操作（如退出）造成资源泄漏
	return device.thread_all_done;
}

INT isRenderRaytracing() {
	return device.render_raytracing > 0 || device.render_raytracing == -2;
}

VOID onResize(FLOAT width, FLOAT height)
{
	isresize = 1;
}

VOID onClose() {
	tmand.~TextureManager();
	if (ptexturePoolImp) free(ptexturePoolImp);
}

EPoint org;
EFTYPE scalex, scaley;

INT enter_once = 1;
INT raytracing_done = 0;
VOID onPaint(HWND hWnd)
{
	if (isresize)
	{
		if (isresize != -1)
		{
			onClose();
		}
		isresize = 0;

		//
		width = getwidth();
		height = getheight();

		device.Resize(width, height);

		org.Set(width, height);
		org /= 2;
		scalex = org.x / 4, scaley = org.x / 4;

		man.setCameraRange(org.x, org.y, scalex, scaley);
	}
	if (isrefresh < 1) {
		return;
	}
	isrefresh = -1;
	// Place draw code here
	EP_SetColor(EP_BLACK);
	EP_ClearDevice();

	//Vertex shader
	man.shaderVertex(NULL);

	//Render in device buffer
	if (device.render_raytracing > 0) {
		if (enter_once < 0) {
			isrefresh = 1;
			//Blt buffer to window buffer
			DWORD * _tango = EP_GetImageBuffer();
			int i, j, index;
			for (i = 0; i < device.width; i++) {
				for (j = 0; j < device.height; j++){
					index = j *  device.width + i;
					if (device.raytracing[index] != EP_BLACK)
					{
						//::SetPixel(memHDC, i, j, device.tango[index]);
						_tango[index] = device.raytracing[index];
					}
				}
			}
			if (device.draw_line > 0) {
				device.drawThreadSplit();
			}
			return;
		}
		enter_once = -1;
		isrefresh = 1;
		device.ClearBeforeRayTracing();
		if (device.thread_count > 0) {
			device.RenderRayTracing(man);
		}
		else {
			device.RenderRayTracing_SingleThread(man);
		}
		//Blt buffer to window buffer
		DWORD * _tango = EP_GetImageBuffer();
		int i, j, index;
		for (i = 0; i < device.width; i++) {
			for (j = 0; j < device.height; j++){
				index = j *  device.width + i;
				if (device.raytracing[index] != EP_BLACK)
				{
					//::SetPixel(memHDC, i, j, device.tango[index]);
					_tango[index] = device.raytracing[index];
				}
			}
		}
		raytracing_done = 1;
	}
	else  {
		if (device.render_raytracing == -2) {
			isrefresh = 1;
			//Blt buffer to window buffer
			DWORD * _tango = EP_GetImageBuffer();
			int i, j, index;
			for (i = 0; i < device.width; i++) {
				for (j = 0; j < device.height; j++) {
					index = j *  device.width + i;
					if (device.raytracing[index] != EP_BLACK)
					{
						//::SetPixel(memHDC, i, j, device.tango[index]);
						_tango[index] = device.raytracing[index];
					}
				}
			}
			if (device.draw_line > 0) {
				device.drawThreadSplit();
			}
			return;
		}
		if (device.draw_oct > 0) {
			device.drawAABB(man, man.octTree.link);
		}
		enter_once = 1;
		if (device.move_light > 0) {
			device.RenderShade(man);
		}
		device.ClearBeforeRender();
		if (device.render_thread > 0) {
			if (man.changed > 0) {
				man.changed--;
				if (device.thread_ready_r) {
					device.ClearBeforeRenderDepth();
					device.RenderThreadReady(man, NULL, NULL, NULL, &device);
				}
			}
			//device.ClearBeforeRenderDepth();
			device.RenderThread(man);
			while (!device.IsThreadDone());
		}
		else {
			device.ClearBeforeRenderDepth();
			device.Render(man, NULL, NULL, NULL);
		}
		if (device.render_mirror > 0) {
			device.RenderMirror(man);
		}
		//Blt buffer to window buffer
		DWORD * _tango = EP_GetImageBuffer();
		int i, j, index;
		for (i = 0; i < device.width; i++) {
			for (j = 0; j < device.height; j++){
				index = j *  device.width + i;
				if (device.tango[index] != EP_BLACK)
				{
					//::SetPixel(memHDC, i, j, device.tango[index]);
					_tango[index] = device.tango[index];
				}
			}
		}
	}

	//BitBlt(hdc, 0, 0, nWidth, nHeight, memHdc, 0, 0, SRCCOPY);
}

#include "../reader/3DS.h"
#include "../reader/OBJ.h"

extern t3DModel g_3DModel[10];

Object3D * cur_op = NULL;

#define MAX_VERTS	100
#define MAX_OBJS	100

#define MAX_STR 100
#define MAX_PAR	10
INT parseParameter(CHAR buffer[], CHAR command[], CHAR parameters[][MAX_STR], INT maxP = MAX_PAR)
{
	int i, j;
	int cp = 0;
	int pp = 0;
	int pc = 0;
	int flag = 0;
	int lspc = 0;
	for (i = 0; buffer[i] != '\0' && (buffer[i] == '\t' || buffer[i] == ' '); i++);
	for (j = i; buffer[j] != '\0'; j++)
	{
		if (buffer[j] == '\n')
		{
			buffer[j] = '\0';
			break;
		}
	}
	for (; buffer[i] != '\0'; i++)
	{
		if (buffer[i] == '\t' || buffer[i] == ' ')
		{
			if (lspc == 0)
			{
				if (flag)
				{
					parameters[pc][pp] = '\0';
					pc++;
					if (maxP && pc >= maxP - 1) {
						break;
					}
					pp = 0;
				}
				else
				{
					command[cp] = '\0';
					flag = 1;
				}
			}
			lspc++;
			continue;
		}
		else
		{
			lspc = 0;
		}
		if (flag)
		{
			parameters[pc][pp++] = buffer[i];
			if (pp >= MAX_STR)
			{
				break;
			}
		}
		else
		{
			command[cp++] = buffer[i];
			if (cp >= MAX_STR)
			{
				break;
			}
		}
	}
	if (flag)
	{
		parameters[pc][pp] = '\0';
	}
	else
	{
		command[cp] = '\0';
		pc = pc - 1;
	}
	return pc + 1;
}
VOID Initialize()
{
	_TextureLocalPoolImp(&textureLocalPoolImp);
	tman.texturePoolImp = &textureLocalPoolImp;
	tman.Init();
	INT tid = tman.addTexture(64, 64, 8);
	tid = tman.addTexture("image/1.jpg");

	man.vobjPoolImp = &vobjPoolImp;
	man.objPoolImp = &objPoolImp;
	man.camPoolImp = &camPoolImp;
	man.lgtPoolImp = &lgtPoolImp;
	man.octPoolImp = &octPoolImp;
	man.group3DPoolImp = &group3DPoolImp;
	_VObjPoolImp(&vobjPoolImp);
	_ObjPoolImp(&objPoolImp, &vobjPoolImp);
	_CamPoolImp(&camPoolImp);
	_LgtPoolImp(&lgtPoolImp);
	_VObjPoolImp(&vobjPoolImp);
	_OctPoolImp(&octPoolImp, &objPoolImp, &vobjPoolImp);
	_Group3DPoolImp(&group3DPoolImp, &objPoolImp, &vobjPoolImp);
	man.Init();


	Camera3D * cam = &man.addCamera(50, 50, 50, 1000, 70, 70);
	man.setCameraRange(500, 240, 126, 126);
	cam->_move(cam, 0, 0, -200);

	man.addLight(9, 100, 300);

	if (0) {
		Object3D *_obj = &man.addObject();
		_obj->addVert(_obj, -10, -10, 10).addVert(_obj, 10, -10, 10).addVert(_obj, -10, 10, 10).addVertA(_obj, 10, 10, 10, -1)._scale(_obj, 5, 5, 5)
			._move(_obj, 0, 100, -200).setColor(_obj, GREEN).setTexture(_obj, tman, 0, 0).setUV(_obj, 0, 0);
		_obj = &man.addObject();
		_obj->addVert(_obj, -10, 0, -10).addVert(_obj, 10, 0, -10).addVert(_obj, -10, 0, 10).addVertA(_obj, 10, 0, 10, -1)._rotate(_obj, 0, 0, 180)
			._scale(_obj, 5, 5, 5)._move(_obj, 250, -40, 250).setColor(_obj, LIGHTGRAY);
	}

	//从文件配置加载
	if (1) {

		FILE * fp = NULL;
		fopen_s(&fp, "scene.json", "r");
		if (!fp) {
			exit(0);
		}
		CHAR buffer[MAX_STR];
		CHAR command[MAX_STR];
		CHAR parameters[MAX_PAR][MAX_STR];
		CHAR attrs[MAX_PAR][MAX_STR];
		int attrCount = 0;
		int paramCount = 0;

		while (!feof(fp)) {
			fgets(buffer, MAX_STR, fp);

			paramCount = parseParameter(buffer, command, parameters);

			if (!strcmp(command, "env"))
			{
				if (paramCount >= 0) {
					while (!feof(fp)) {
						fgets(buffer, MAX_STR, fp);

						attrCount = parseParameter(buffer, command, attrs);

						if (!strcmp(command, "env")) {
							break;
						}
						/////////////////////////////////////////////////////////////
						else if (!strcmp(command, "render_mode")) {
							if (attrCount > 0) {
								device.render_mode = atoi(attrs[0]);
							}
						}
						else if (!strcmp(command, "samples_per_pixel")) {
							if (attrCount > 0) {
								device.samples_per_pixel = atoi(attrs[0]);
							}
						}
						else if (!strcmp(command, "max_depth")) {
							if (attrCount > 0) {
								device.max_depth = atoi(attrs[0]);
							}
						}
						else if (!strcmp(command, "aspect_ratio")) {
							if (attrCount > 0) {
								device.aspect_ratio = atof(attrs[0]);
							}
						}
						else if (!strcmp(command, "aperture")) {
							if (attrCount > 0) {
								device.aperture = atof(attrs[0]);
							}
						}
						else if (!strcmp(command, "type")) {
							if (attrCount > 0) {
								device.type = atoi(attrs[0]);
							}
						}
						else if (!strcmp(command, "cam_type")) {
							if (attrCount > 0) {
								device.cam_type = atoi(attrs[0]);
							}
						}
						else if (!strcmp(command, "bvh")) {
							if (attrCount > 0) {
								device.bvh = atoi(attrs[0]);
							}
						}
						/////////////////////////////////////////////////////////////
						else if (!strcmp(command, "split")) {
							device.draw_line = 1;
						}
						else if (!strcmp(command, "mirror")) {
							device.render_mirror = 1;
						}
						else if (!strcmp(command, "count")) {
							if (attrCount > 0) {
								device.raytracing_count = atoi(attrs[0]);
							}
						}
						else if (!strcmp(command, "light")) {
							if (attrCount > 0) {
								device.light_count = atoi(attrs[0]);
								if (device.light_count < 0) {
									device.light_count = 0;
								}
								else if (device.light_count >= MAX_LIGHT) {
									device.light_count = MAX_LIGHT;
								}
							}
						}
						else if (!strcmp(command, "raster")) {
							if (attrCount > 1) {
								device.thread_w = atoi(attrs[0]);
								device.thread_h = atoi(attrs[1]);
								//maxium thread exceeded
								if (device.thread_w < 0) {
									device.thread_w = 0;
								}
								if (device.thread_h < 0) {
									device.thread_h = 0;
								}
								else if (device.thread_w * device.thread_h > MAX_OBJ3D_THREAD) {
									device.thread_w = MAX_OBJECT;
									device.thread_h = MAX_OBJECT;
								}
							}
							else if (attrCount > 0) {
								device.thread_w = atoi(attrs[0]);
								//maxium thread exceeded
								if (device.thread_w < 0) {
									device.thread_w = 0;
								}
								else if (device.thread_w * device.thread_w > MAX_OBJ3D_THREAD) {
									device.thread_w = MAX_OBJECT;
								}
								device.thread_h = device.thread_w;
							}
						}
						else if (!strcmp(command, "thread")) {
							if (attrCount > 1) {
								device.thread_count = atoi(attrs[0]);
								device.thread_count_h = atoi(attrs[1]);
								//maxium thread exceeded
								if (device.thread_count < 0) {
									device.thread_count = 0;
								}
								if (device.thread_count_h < 0) {
									device.thread_count_h = 0;
								}
								else if (device.thread_count * device.thread_count_h > MAX_OBJ3D_THREAD) {
									device.thread_count = 1;
									device.thread_count_h = device.thread_count;
								}
							}
							else if (attrCount > 0) {
								device.thread_count = atoi(attrs[0]);
								//maxium thread exceeded
								if (device.thread_count < 0) {
									device.thread_count = 0;
								}
								else if (device.thread_count * device.thread_count > MAX_OBJ3D_THREAD) {
									device.thread_count = 1;
								}
								device.thread_count_h = device.thread_count;
							}
						}
					}
				}
			}
			else if (!strcmp(command, "camera"))
			{
				if (paramCount > 0) {
					Camera3D * cam = NULL;
					while (!feof(fp)) {
						fgets(buffer, MAX_STR, fp);

						attrCount = parseParameter(buffer, command, attrs);

						if (!cam) {
							if (!strcmp(parameters[0], "shadow")) {
								cam = &man.addShadowCamera();
							}
							else if (!strcmp(parameters[0], "reflection")) {
								cam = &man.addReflectionCamera();
							}
						}
						if (!strcmp(command, "camera")) {
							break;
						}
						else if (!strcmp(command, "param")) {
							EFTYPE param[6];
							for (int i = 0; i < 6; i++) {
								if (i < attrCount) {
									param[i] = atof(attrs[i]);
								}
								else {
									param[i] = 0;
								}
							}
							if (!cam) {
								cam = &man.addCamera(param[0], param[1], param[2], param[3], param[4], param[5]);
							}
						}
						else if (!strcmp(command, "move")) {
							EFTYPE param[3];
							for (int i = 0; i < 3; i++) {
								if (i < attrCount) {
									param[i] = atof(attrs[i]);
								}
								else {
									param[i] = 0;
								}
							}
							if (cam) {
								cam->_move(cam, param[0], param[1], param[2]);
							}
						}
					}
				}
			}
			else if (!strcmp(command, "light"))
			{
				if (paramCount > 0) {
					Light3D * lgt = NULL;
					while (!feof(fp)) {

						fgets(buffer, MAX_STR, fp);

						attrCount = parseParameter(buffer, command, attrs);

						if (!strcmp(command, "light")) {
							break;
						}
						else if (!strcmp(command, "param")) {
							EFTYPE param[3];
							for (int i = 0; i < 3; i++) {
								if (i < attrCount) {
									param[i] = atof(attrs[i]);
								}
								else {
									param[i] = 0;
								}
							}
							if (!lgt) {
								lgt = &man.addLight(param[0], param[1], param[2]);
							}
						}
						else if (!strcmp(command, "move")) {
							EFTYPE param[3];
							for (int i = 0; i < 3; i++) {
								if (i < attrCount) {
									param[i] = atof(attrs[i]);
								}
								else {
									param[i] = 0;
								}
							}
							if (lgt) {
								lgt->_move(lgt, param[0], param[1], param[2]);
							}
						}
					}
				}
			}
			else if (!strcmp(command, "texture"))
			{
				if (paramCount > 0) {
					INT textureID = 0;
					while (!feof(fp)) {

						fgets(buffer, MAX_STR, fp);

						attrCount = parseParameter(buffer, command, attrs);

						if (!strcmp(command, "texture")) {
							break;
						}
						else if (!strcmp(command, "param")) {
							EFTYPE param[3];
							for (int i = 0; i < 3; i++) {
								if (i < attrCount) {
									param[i] = atof(attrs[i]);
								}
								else {
									param[i] = 0;
								}
							}
							if (!textureID) {
								textureID = tman.addTexture(param[0], param[1], param[2]);
							}
						}
						else if (!strcmp(command, "url")) {
							if (!textureID) {
								textureID = tman.addTexture(attrs[0]);
							}
						}
					}
				}
			}
			else if (!strcmp(command, "break")) {
				//break loading json for debug
				break;
			}
			else if (!strcmp(command, "object"))
			{
				if (paramCount > 1) {
					INT vertextType = 0;
					INT isUV = 0;
					EFTYPE uv[2];
					INT isMove = 0;
					EFTYPE move[3];
					INT isRotate = 0;
					EFTYPE rotate[3];
					INT isScale = 0;
					EFTYPE scale[3];
					INT normalType = 0;
					ECOLOR color = EP_BLACK;
					ECOLOR lineColor = EP_BLACK;
					INT textureID = 0;
					INT textureType = 0;

					INT isMaxx = 0;
					INT maxx[3];
					INT isMovex = 0;
					EFTYPE movex[3];

					INT	round = 0;
					INT count = 0;
					EFTYPE diameter = 0;
					INT backfaceculling = 0;
					INT revert = 0;
					INT uniqueID = 0;

					INT stepU = 0;
					INT stepV = 0;

					Vert3D verts[MAX_VERTS];
					INT vertsIndex = 0;

					CHAR url[MAX_STR];

					while (!feof(fp)) {

						fgets(buffer, MAX_STR, fp);

						attrCount = parseParameter(buffer, command, attrs);

						if (!strcmp(command, "object")) {
							Object3D * objs[MAX_OBJS];
							Object3D * obj = NULL;
							INT index = 0;

							INT type = 0;
							EFTYPE parameter = 0;
							if (!strcmp(parameters[1], "normal")) {
								type = 1;
								parameter = 1;
								if (paramCount > 2) {
									parameter = atoi(parameters[2]);
								}
							}
							else if (!strcmp(parameters[1], "reflection")) {
								type = 2;
								if (paramCount > 2) {
									parameter = atof(parameters[2]);
								}
							}
							else if (!strcmp(parameters[1], "transparent")) {
								type = 3;
								if (paramCount > 2) {
									parameter = atof(parameters[2]);
								}
							}
							if (!strcmp(parameters[0], "normal")) {
								if (type == 2) {
									obj = &man.addReflectionObject(parameter);
								}
								else if (type == 3) {
									obj = &man.addTransparentObject(parameter);
								}
								else {
									obj = &man.addObjectI((INT)parameter);
								}
								if (vertextType) {
									obj->setVertexType(obj, vertextType);
								}

								for (int i = 0; i < vertsIndex; i++) {
									obj->addVertA(obj, verts[i].x, verts[i].y, verts[i].z, verts[i].anti);
								}

								objs[index++] = obj;
								if (index > MAX_OBJS) {
									index = MAX_OBJS - 1;
								}
							}
							if (!strcmp(parameters[0], "3ds")) {
								CLoad3DS loader;
								INT loadIndex = 0;
								loader.Init(url, loadIndex);

								for (int i = 0; i < loader.m_3DModel.numOfObjects; i++) {
									t3DObject & object = loader.m_3DModel.pObject.at(i);

									if (type == 2) {
										obj = &man.addReflectionObject(parameter);
									}
									else if (type == 3) {
										obj = &man.addTransparentObject(parameter);
									}
									else {
										obj = &man.addObjectI((INT)parameter);
									}

									if (vertextType) {
										obj->setVertexType(obj, vertextType);
									}

									for (int j = 0; j < object.numOfVerts; j++) {
										obj->addIndiceN(obj, object.pVerts[j].x, object.pVerts[j].z, object.pVerts[j].y, object.pNormals[j].x, object.pNormals[j].z, object.pNormals[j].y);
									}
									INT anti_n = 1;
									for (int j = 0; j < object.numOfFaces; j++) {
										obj->setIndiceI(obj, object.pFaces[j].vertIndex[0], object.pFaces[j].vertIndex[2], object.pFaces[j].vertIndex[1], anti_n);
										if (revert) {
											anti_n = -anti_n;
										}
									}

									if (color) {
										obj->setColor(obj, color);
									}
									else {
										if (loader.m_3DModel.pMaterials.size() > object.materialID) {
											obj->setColorB(obj, loader.m_3DModel.pMaterials[object.materialID].color);
											if (object.bHasTexture) {
												int start = 0;
												for (int j = 0; url[j] != '\0'; j++) {
													if (url[j] == '/') {
														start = j;
													}
												}
												url[start] = '/';
												for (int j = 0; j + start + 1 < 100; j++) {
													url[j + start + 1] = loader.m_3DModel.pMaterials[object.materialID].strFile[j];
													if (url[j + start + 1] == '\0') {
														break;
													}
												}
												int tID = tman.addTexture(url);
												obj->setTexture(obj, tman, tID, 3);
											}
										}
									}

									objs[index++] = obj;
									if (index > MAX_OBJS) {
										index = MAX_OBJS - 1;
									}
								}
							}
							else if (!strcmp(parameters[0], "obj")) {
								ObjParser objParser;
								int result = objParser.ParseEx(url, buffer, command, parameters, attrs, MAX_STR, parseParameter);

								if (result > 0) {

									if (type == 2) {
										obj = &man.addReflectionObject(parameter);
									}
									else if (type == 3) {
										obj = &man.addTransparentObject(parameter);
									}
									else {
										obj = &man.addObjectI((INT)parameter);
									}
									if (vertextType) {
										obj->setVertexType(obj, vertextType);
									}


									int vIndex, tIndex, nIndex;
									int _vIndex, _tIndex, _nIndex;
									for (int i = 0; i < objParser.mFaceCount; i++)
									{
										int anti = 1;
										for (int j = 0; j < 4; j++) {
											vIndex = objParser.faces[i].vIndex[j] - 1;
											tIndex = objParser.faces[i].tIndex[j] - 1;
											nIndex = objParser.faces[i].nIndex[j] - 1;

											if (vIndex >= 0) {
												if (j == 3) {
													//it's very strange that when the polygon has 4 verts
													//the verts are in order as the following:
													//1 -> 2 -> 3 ( -> 1 -> 3 -> ) -> 4
													//there must be verts 1 and 3 (in the brace) 
													//when operating the 4th vert
													//and the verts_type must be triangle insted of triangle strip
													for (int k = 0; k < 4; k++) {
														if (k == 0 || k == 2) {
															_vIndex = objParser.faces[i].vIndex[k] - 1;
															_tIndex = objParser.faces[i].tIndex[k] - 1;
															_nIndex = objParser.faces[i].nIndex[k] - 1;

															if (_vIndex >= 0) {
																if (_nIndex >= 0) {
																	obj->addVertN(obj, objParser.vertices[_vIndex].x, objParser.vertices[_vIndex].y, objParser.vertices[_vIndex].z,
																		anti * objParser.normals[_nIndex].x, anti * objParser.normals[_nIndex].y, anti * objParser.normals[_nIndex].z);
																}
																else {
																	obj->addVertA(obj, objParser.vertices[_vIndex].x, objParser.vertices[_vIndex].y, objParser.vertices[_vIndex].z, anti);
																}
															}
															//anti = -anti;
														}
													}
												}
												if (nIndex >= 0) {
													obj->addVertN(obj, objParser.vertices[vIndex].x, objParser.vertices[vIndex].y, objParser.vertices[vIndex].z,
														anti * objParser.normals[nIndex].x, anti * objParser.normals[nIndex].y, anti * objParser.normals[nIndex].z);
												}
												else {
													obj->addVertA(obj, objParser.vertices[vIndex].x, objParser.vertices[vIndex].y, objParser.vertices[vIndex].z, anti);
												}
											}
											//anti = -anti;
										}
									}
									objs[index++] = obj;
								}
							}
							else if (!strcmp(parameters[0], "group")) {

								if (isMaxx) {

									Group3D& gp = man.addGroup();

									for (int i = 0; i < maxx[0]; i++) {
										for (int j = 0; j < maxx[1]; j++) {

											if (type == 2) {
												obj = &man.addReflectionObject(parameter);
											}
											else if (type == 3) {
												obj = &man.addTransparentObject(parameter);
											}
											else {
												obj = &man.addObjectI((INT)parameter);
											}


											for (int i = 0; i < vertsIndex; i++) {
												obj->addVertA(obj, verts[i].x, verts[i].y, verts[i].z, verts[i].anti);
											}


											if (isMovex) {
												if (isMovex == 1) {
													obj->_move(obj, 0, movex[0] * i, movex[2] * j);
												}
												else if (isMovex == 2) {
													obj->_move(obj, movex[0] * i, 0, movex[2] * j);
												}
												else if (isMovex == 3) {
													obj->_move(obj, movex[0] * i, movex[1] * j, 0);
												}
											}

											objs[index++] = obj;
											if (index > MAX_OBJS) {
												index = MAX_OBJS - 1;
											}
										}
									}
									man.endGroup();
								}
							}
							else if (!strcmp(parameters[0], "sphere")) {

								if (count && round) {
									EFTYPE x_1, r_1, x_2, r_2;
									EFTYPE r = diameter;
									EFTYPE p_1 = PI / ((EFTYPE)round);
									EFTYPE p_2 = 2 * PI / ((EFTYPE)round);
									for (int k = 0; k < count; k++) {
										Group3D& gp = man.addGroup();
										for (int i = 0; i < round; i++) {
											x_1 = r * cos(i * p_1);
											r_1 = r * sin(i * p_1);
											x_2 = r * cos((i + 1) * p_1);
											r_2 = r * sin((i + 1) * p_1);

											if (type == 2) {
												obj = &man.addReflectionObject(parameter);
											}
											else if (type == 3) {
												obj = &man.addTransparentObject(parameter);
											}
											else {
												obj = &man.addObjectI((INT)parameter);
											}

											obj->addVert(obj, x_1, 0, -r_1).addVert(obj, x_2, 0, -r_2);
											for (int j = 1; j < round; j++) {
												obj->addVert(obj, x_1, r_1 * sin(j * p_2), -r_1 * cos(j * p_2))
													.addVertA(obj, x_2, r_2 * sin(j * p_2), -r_2 * cos(j * p_2), -1);
											}
											obj->addVert(obj, x_1, 0, -r_1).addVertA(obj, x_2, 0, -r_2, -1).setCenter(obj, 0, 0, 0);

											objs[index++] = obj;
											if (index > MAX_OBJS) {
												index = MAX_OBJS - 1;
											}
										}
									}
									man.endGroup();
								}
							}
							else if (!strcmp(parameters[0], "earth")) {
								if (type == 2) {
									obj = &man.addReflectionObject(parameter);
								}
								else if (type == 3) {
									obj = &man.addTransparentObject(parameter);
								}
								else {
									obj = &man.addObjectI((INT)parameter);
								}
								if (vertextType) {
									obj->setVertexType(obj, vertextType);
								}

								double ustep = 1 / (double)stepU, vstep = 1 / (double)stepV;
								double u = 0, v = 0;

								//绘制中间四边形组
								u = 0, v = 0;
								for (int i = 0; i < stepV; i++) //纬度v
								{
									for (int j = 0; j < stepU; j++) //经度u
									{
										//if (i == 0 || i == vStepNum - 1) continue;

										Point3d a = getSpherePoint(u, v);
										Point3d b = getSpherePoint(u + ustep, v);
										Point3d c = getSpherePoint(u + ustep, v + vstep);
										Point3d d = getSpherePoint(u, v + vstep);

										float u0 = 1 - 1.0f / stepU * j;
										float u1 = 1 - 1.0f / stepU * (j + 1);
										float v0 = 1 - 1.0f / stepV * i;
										float v1 = 1 - 1.0f / stepV * (i + 1);

										//glBegin(GL_TRIANGLE_FAN);
										//glNormal3f(0.0f, 0.0f, 1.0f);
										//glTexCoord2f(u0, v0); glVertex3d(a.x, a.y, a.z);
										//glTexCoord2f(u1, v0); glVertex3d(b.x, b.y, b.z);
										//glTexCoord2f(u1, v1); glVertex3d(c.x, c.y, c.z);
										//glTexCoord2f(u0, v1); glVertex3d(d.x, d.y, d.z);
										//glEnd();

										obj->setUV(obj, u0, v0).addVert(obj, a.x, a.y, a.z)
											.setUV(obj, u1, v0).addVert(obj, b.x, b.y, b.z)
											.setUV(obj, u1, v1).addVert(obj, c.x, c.y, c.z)
											.setUV(obj, u0, v0).addVert(obj, a.x, a.y, a.z)
											.setUV(obj, u1, v1).addVert(obj, c.x, c.y, c.z)
											.setUV(obj, u0, v1).addVert(obj, d.x, d.y, d.z);

										u += ustep;
									}
									v += vstep;
								}

								objs[index++] = obj;
								if (index > MAX_OBJS) {
									index = MAX_OBJS - 1;
								}
							}
							else if (!strcmp(parameters[0], "file")) {

								FILE * _fp = NULL;
								fopen_s(&_fp, url, "r");
								if (_fp) {

									static CHAR largeBuffer[102400];
									static CHAR largeParameters[4][5000][100];
									INT teapotPositions = 0;
									INT teapotNormals = 0;
									INT teapotIndices = 0;
									INT teapotPositionsNum = 0;
									INT teapotNormalsNum = 0;
									INT teapotIndicesNum = 0;
									INT curIndex = 0;
									while (!feof(_fp)) {
										fgets(largeBuffer, 102400, _fp);

										if (curIndex > 3) {
											curIndex = 3;
										}

										attrCount = parseParameter(largeBuffer, command, largeParameters[curIndex], 5000);

										if (!strcmp(command, "teapotPositions")) {
											teapotPositions = curIndex;
											teapotPositionsNum = attrCount;
											curIndex++;
										}
										else if (!strcmp(command, "teapotNormals")) {
											teapotNormals = curIndex;
											teapotNormalsNum = attrCount;
											curIndex++;
										}
										else if (!strcmp(command, "teapotIndices")) {
											teapotIndices = curIndex;
											teapotIndicesNum = attrCount;
											curIndex++;
										}
									}
									fclose(_fp);

									if (type == 2) {
										obj = &man.addReflectionObject(parameter);
									}
									else if (type == 3) {
										obj = &man.addTransparentObject(parameter);
									}
									else {
										obj = &man.addObjectI((INT)parameter);
									}

									if (vertextType) {
										obj->setVertexType(obj, vertextType);
									}

									int normal = -1;
									int vertex_count = 0;
									int triangle_count = 0;
									for (int i = 0; i <= teapotPositionsNum - 3; i += 3) {
										vertex_count++;
										obj->addIndiceN(obj, atof(largeParameters[teapotPositions][i]), atof(largeParameters[teapotPositions][i + 1]), atof(largeParameters[teapotPositions][i + 2])
											, atof(largeParameters[teapotNormals][i]), atof(largeParameters[teapotNormals][i + 1]), atof(largeParameters[teapotNormals][i + 2]));
									}
									for (int i = 0; i <= teapotIndicesNum - 3; i += 3) {
										triangle_count++;
										obj->setIndice(obj, atof(largeParameters[teapotIndices][i]), atof(largeParameters[teapotIndices][i + 1]), atof(largeParameters[teapotIndices][i + 2]));
									}

									objs[index++] = obj;
									if (index > MAX_OBJS) {
										index = MAX_OBJS - 1;
									}
								}
							}

							for (int i = 0; i < index; i++) {
								obj = objs[i];
								if (!obj) {
									break;
								}

								if (isMove) {
									obj->_move(obj, move[0], move[1], move[2]);
								}
								if (isRotate) {
									obj->_rotate(obj, rotate[0], rotate[1], rotate[2]);
								}
								if (isScale) {
									obj->_scale(obj, scale[0], scale[1], scale[2]);
								}
								if (normalType) {
									obj->setNormalType(obj, normalType);
								}
								if (color) {
									obj->setColor(obj, color);
								}
								if (textureID) {
									obj->setTexture(obj, tman, textureID, textureType);
								}
								if (isUV) {
									obj->setUV(obj, uv[0], uv[1]);
								}
								if (backfaceculling) {
									obj->setBackfaceCulling(obj, backfaceculling);
								}
								if (uniqueID) {
									((Obj3D *)obj)->uniqueID = uniqueID;
								}
								cur_op = obj;
							}
							break;
						}
						else if (!strcmp(command, "verts")) {
							EFTYPE param[3];
							int anti = 1;
							for (int i = 0; i < 3; i++) {
								if (i < attrCount) {
									param[i] = atof(attrs[i]);
								}
								else {
									param[i] = 0;
								}
							}
							if (attrCount >= 4) {
								anti = atoi(attrs[3]);
							}
							verts[vertsIndex].set(param[0], param[1], param[2]);
							verts[vertsIndex].anti = anti;
							vertsIndex++;
							if (vertsIndex > MAX_VERTS) {
								vertsIndex = MAX_VERTS - 1;
							}
						}
						else if (!strcmp(command, "rotate")) {
							for (int i = 0; i < 3; i++) {
								if (i < attrCount) {
									rotate[i] = atof(attrs[i]);
									isRotate++;
								}
								else {
									rotate[i] = 0;
								}
							}
						}
						else if (!strcmp(command, "scale")) {
							for (int i = 0; i < 3; i++) {
								if (i < attrCount) {
									scale[i] = atof(attrs[i]);
									isScale++;
								}
								else {
									scale[i] = 0;
								}
							}
						}
						else if (!strcmp(command, "move")) {
							for (int i = 0; i < 3; i++) {
								if (i < attrCount) {
									move[i] = atof(attrs[i]);
									isMove++;
								}
								else {
									move[i] = 0;
								}
							}
						}
						else if (!strcmp(command, "uv")) {
							for (int i = 0; i < 2; i++) {
								if (i < attrCount) {
									uv[i] = atof(attrs[i]);
									isUV++;
								}
								else {
									uv[i] = 0;
								}
							}
						}
						else if (!strcmp(command, "texture")) {
							if (attrCount > 0) {
								textureID = atoi(attrs[0]);
								if (attrCount > 1) {
									textureType = atoi(attrs[1]);
								}
							}
						}
						else if (!strcmp(command, "color")) {
							if (attrCount > 0) {
								color = EP_ColorConvert(attrs[0]);
							}
						}
						else if (!strcmp(command, "line_color")) {
							if (attrCount > 0) {
								lineColor = EP_ColorConvert(attrs[0]);
							}
						}
						else if (!strcmp(command, "url")) {
							if (attrCount > 0) {
								memcpy(url, attrs[0], MAX_STR);
							}
						}
						else if (!strcmp(command, "vertext_type")) {
							if (attrCount > 0) {
								vertextType = atoi(attrs[0]);
							}
						}
						else if (!strcmp(command, "normal_type")) {
							if (attrCount > 0) {
								normalType = atoi(attrs[0]);
							}
						}
						else if (!strcmp(command, "maxx")) {
							for (int i = 0; i < 3; i++) {
								if (i < attrCount) {
									maxx[i] = atoi(attrs[i]);
									isMaxx++;
								}
								else {
									maxx[i] = 0;
								}
							}
						}
						else if (!strcmp(command, "movex")) {
							for (int i = 0; i < 3; i++) {
								if (i < attrCount) {
									movex[i] = atoi(attrs[i]);
									if (EP_ISZERO(movex[i])) {
										isMovex = i + 1;
									}
								}
								else {
									movex[i] = 0;
								}
							}
						}
						else if (!strcmp(command, "round")) {
							if (attrCount > 0) {
								round = atoi(attrs[0]);
							}
						}
						else if (!strcmp(command, "count")) {
							if (attrCount > 0) {
								count = atoi(attrs[0]);
							}
						}
						else if (!strcmp(command, "diameter")) {
							if (attrCount > 0) {
								diameter = atof(attrs[0]);
							}
						}
						else if (!strcmp(command, "revert")) {
							if (attrCount > 0) {
								revert = atof(attrs[0]);
							}
						}
						else if (!strcmp(command, "backfaceculling")) {
							if (attrCount > 0) {
								backfaceculling = atof(attrs[0]);
							}
						}
						else if (!strcmp(command, "step")) {
							if (attrCount > 0) {
								stepV = stepU = atof(attrs[0]);
								if (attrCount > 1) {
									stepV = atof(attrs[1]);
								}
							}
						}
						else if (!strcmp(command, "id")) {
							if (attrCount > 0) {
								uniqueID = atof(attrs[0]);
							}
						}
					}
				}
			}
		}
		fclose(fp);
	}

	//这里模拟cuda的host-device操作，将texture从host复制到device，并修改obj的texture指针
	ptexturePoolImp = (TexturePoolImp*)malloc(sizeof(TexturePoolImp));
	TexturePoolImp& texturePoolImp = *ptexturePoolImp;
	//tmand.texturePoolImp = &texturePoolImp;
	//make pool copy, make sure TextureLocalPoolImp and TexturePoolImp are the same size
	//memcpy(&texturePoolImp, &textureLocalPoolImp, sizeof(TexturePoolImp));
	//texture pointer reset
	TextureLocalPoolImp textPoolBackup;
	for (int i = 0; i < MAX_TEXTURE; i++) {
		textPoolBackup.pool[i].texture = NULL;
	}
	//texture copy
	for (int i = 0; i < MAX_TEXTURE; i++) {
		TextureLocal *textureLocal = &textureLocalPoolImp.pool[i];
		if (!textureLocal) {
			continue;
		}
		if (textureLocal->texture != NULL && textureLocal->width > 0 && textureLocal->height > 0) {
			DWORD * texture = new DWORD[textureLocal->width * textureLocal->height];
			memcpy(texture, textureLocal->texture, sizeof(DWORD) * textureLocal->width * textureLocal->height);
			textPoolBackup.pool[i].texture = texture;
			textPoolBackup.pool[i].width = textureLocal->width;
			textPoolBackup.pool[i].height = textureLocal->height;
			textPoolBackup.pool[i].uniqueID = textureLocal->uniqueID;
		}
	}
	memcpy(&texturePoolImp, &textPoolBackup, sizeof(TextureLocalPoolImp));

	tmand.texturePoolImp = &texturePoolImp;
	tmand.Init();
	tmand.Reload();
	//遍历物体，替换texture指针
	Obj3D * obj = man.objs.link;
	if (obj) {
		do {
			if (obj->texture) {
				for (int i = 0; i < MAX_TEXTURE; i++) {
					if (obj->texture == textureLocalPoolImp.pool[i].texture) {
						if (texturePoolImp.pool[i].texture != NULL) {
							obj->texture = texturePoolImp.pool[i].texture;
						}
					}
				}
			}
			obj = man.objs.next(&man.objs, obj);
		} while (obj && obj != man.objs.link);
	}
	obj = man.refl.link;
	if (obj) {
		do {
			if (obj->texture) {
				for (int i = 0; i < MAX_TEXTURE; i++) {
					if (obj->texture == textureLocalPoolImp.pool[i].texture) {
						if (texturePoolImp.pool[i].texture != NULL) {
							obj->texture = texturePoolImp.pool[i].texture;
						}
					}
				}
			}
			obj = man.refl.next(&man.refl, obj);
		} while (obj && obj != man.refl.link);
	}
	obj = man.tras.link;
	if (obj) {
		do {
			if (obj->texture) {
				for (int i = 0; i < MAX_TEXTURE; i++) {
					if (obj->texture == textureLocalPoolImp.pool[i].texture) {
						if (texturePoolImp.pool[i].texture != NULL) {
							obj->texture = texturePoolImp.pool[i].texture;
						}
					}
				}
			}
			obj = man.tras.next(&man.tras, obj);
		} while (obj && obj != man.tras.link);
	}


	if (0) {
		Object3D* _obj = &man.addObject();
		_obj->addVert(_obj, -10, -10, 10).addVert(_obj, 10, -10, 10).addVert(_obj, -10, 10, 10).addVertA(_obj, 10, 10, 10, -1)._scale(_obj, 5, 5, 5)
			._move(_obj, 0, 100, -200).setColor(_obj, GREEN).setTextureD(_obj, tmand, 1, 0).setUV(_obj, 0, 0);
		_obj = &man.addObject();
		_obj->addVert(_obj, -10, 0, -10).addVert(_obj, 10, 0, -10).addVert(_obj, -10, 0, 10).addVertA(_obj, 10, 0, 10, -1)._rotate(_obj, 0, 0, 180)
			._scale(_obj, 5, 5, 5)._move(_obj, 150, -40, 250).setColor(_obj, LIGHTGRAY);

		_obj = &man.addObject();
		_obj->addVert(_obj, -10, -10, 10).addVert(_obj, 10, -10, 10).addVert(_obj, -10, 10, 10).addVertA(_obj, 10, 10, 10, -1)._scale(_obj, 5, 5, 5)
			._move(_obj, 0, 100, -100).setColor(_obj, GREEN).setTextureD(_obj, tmand, 1, 0).setUV(_obj, 0, 0);

		_obj = &man.addObject();
		_obj->addVert(_obj, -10, -10, 10).addVert(_obj, 10, -10, 10).addVert(_obj, -10, 10, 10).addVertA(_obj, 10, 10, 10, -1)._scale(_obj, 5, 5, 5)
			._move(_obj, 0, 100, -0).setColor(_obj, GREEN).setTextureD(_obj, tmand, 1, 0).setUV(_obj, 0, 0);

		_obj = &man.addObject();
		_obj->addVert(_obj, -10, -10, 10).addVert(_obj, 10, -10, 10).addVert(_obj, -10, 10, 10).addVertA(_obj, 10, 10, 10, -1)._scale(_obj, 5, 5, 5)
			._move(_obj, 0, 100, 100).setColor(_obj, GREEN).setTextureD(_obj, tmand, 1, 0).setUV(_obj, 0, 0);
	}


	tman.~TextureLocalManager();

	man.initialized = 1;

	man.rotateCamera(0.6, -104, 0);
	//do this after all done
	man.createOctTree();

	////////////////////////////////////////////////////////////
	_HittablePoolImp(&hittablePoolImp);
	_HittableMan(&world.objects, 0, &hittablePoolImp);
	_SpherePoolImp(&spherePoolImp);
	_SphereMan(&sphereMan, 0, &spherePoolImp);
	_LambertianPoolImp(&lambertianPoolImp);
	_LambertianMan(&lambertianMan, 0, &lambertianPoolImp);
	_MetalPoolImp(&metalPoolImp);
	_MetalMan(&metalMan, 0, &metalPoolImp);
	_DielectricPoolImp(&dielectricPoolImp);
	_DielectricMan(&dielectricMan, 0, &dielectricPoolImp);
	_BvhNodePoolImp(&bvhNodePoolImp);
	_BvhNodeMan(&bvhNodeMan, 0, &bvhNodePoolImp);
	device.man = &man;
	device.world = &world;
	device.sphereMan = &sphereMan;
	device.rtcam = &rtcam;
	//device.samples_per_pixel = 10;
	//device.max_depth = 5;
	//device.aspect_ratio = 1.5;
	//device.aperture = 2.0;
	device.lambertianMan = &lambertianMan;
	device.metalMan = &metalMan;
	device.dielectricMan = &dielectricMan;
	device.bvhNodeMan = &bvhNodeMan;
	//0: real world 1: scene without material 2: scene with material 3: random scene
	//device.type = 0;
	//0: real camera 1:default camere 2;camera wide fov 3;camera narrow fov 
	//4: camera defocus blur 5: camera defocus blur narrow fov 6: camera defocus blur wide fov
	//device.cam_type = 0;
	//you can try 2-0 or 3-5 or 3-6
	//use bvh
	//device.bvh = 1;
	srand(time(NULL));
	device.init_hittable();
	////////////////////////////////////////////////////////////
}

EFTYPE scale = 10.0;
INT is_control = 0;
VOID onScroll(FLOAT delta) {
	if (is_control) {
		if (delta > 0) {
			scale += 1.0;
		}
		else {
			scale -= 1.0;
		}
		if (scale <= 0) {
			scale = 1.0;
		}
	}
	else {
		if (device.move_light > 0) {
			if (delta > 0) {
				man.moveLight(0, 0, scale);
			}
			else {
				man.moveLight(0, 0, -scale);
			}
		}
		else if (device.move_trans > 0) {

			if (delta > 0) {
				Obj3D * obj = man.objs.link;
				if (obj) {
					do {

						obj->_move(obj, 0, 0, scale);


						obj = man.objs.next(&man.objs, obj);
					} while (obj && obj != man.objs.link);
				}
			}
			else {
				Obj3D * obj = man.objs.link;
				if (obj) {
					do {

						obj->_move(obj, 0, 0, -scale);


						obj = man.objs.next(&man.objs, obj);
					} while (obj && obj != man.objs.link);
				}
			}
		}
		else {
			if (delta > 0) {
				man.moveCamera(0, 0, scale);
			}
			else {
				man.moveCamera(0, 0, -scale);
			}
		}

		isrefresh = 1;
	}
}

EPointF menu;
VOID onMenu(FLOAT x, FLOAT y, INT mode)
{
	if (mode == 1) // mouse down
	{
		menu.X = x;
		menu.Y = y;
	}
	else if (mode == 2) // mouse move
	{
		if (EP_NTZERO(menu.X) && EP_NTZERO(menu.Y))
		{
			if (device.move_light > 0) {
				man.moveLight(-(x - menu.X) / scale, -(y - menu.Y) / scale, 0);
			}
			else {
				man.moveCamera(-(x - menu.X) / scale, -(y - menu.Y) / scale, 0);
			}
			menu.X = x;
			menu.Y = y;

			isrefresh = 1;
		}
	}
	else	// mouse up
	{
		menu.X = 0;
		menu.Y = 0;
	}
}

EPointF drag;
VOID onDrag(FLOAT x, FLOAT y, INT mode)
{
	if (mode == 1) // mouse down
	{
		drag.X = x;
		drag.Y = y;
	}
	else if (mode == 2) // mouse move
	{
		if (EP_NTZERO(drag.X) && EP_NTZERO(drag.Y))
		{
			if (device.move_light > 0) {
				man.rotateLight(-(y - drag.Y) / scale, (x - drag.X) / scale, 0);
			}
			else {
				man.rotateCamera(-(y - drag.Y) / scale, (x - drag.X) / scale, 0);
			}
			drag.X = x;
			drag.Y = y;

			isrefresh = 1;
		}
	}
	else	// mouse up
	{
		drag.X = 0;
		drag.Y = 0;
	}
}

INT lean = 0;
VOID onTimer()
{
	return;
	int count = 0;
	Obj3D * obj = man.objs.link;
	if (obj) {
		//obj->rotate(0, 10, 0);
	}
	obj = man.objs.prev(&man.objs, man.objs.link);
	if (obj) {
		obj->_rotate(obj, 0, 10, 0);
		//obj->scale(1.05, 1.05, 1.05);
	}

	obj = man.tras.link;
	if (obj) {
		do {

			obj->_rotate(obj, 0, 10, 0);
			/*
			obj->move(0, -5, 0);

			if (obj->center_w.y + 30 < 0) {
				obj->move(0, 50, 0);
			}
			*/

			obj = man.tras.next(&man.tras, obj);
		} while (obj && obj != man.tras.link);
	}
}
VOID onKeyUp(WPARAM wParam) {
	switch (wParam) {
	case VK_CONTROL:
		is_control = 0;
		break;
	case 'Q':
		if (lean) {
			man.rotateCamera(0, 0, -lean);
			lean = 0;
		}
		break;
	case 'E':
		if (lean) {
			man.rotateCamera(0, 0, -lean);
			lean = 0;
		}
		break;
	}

	isrefresh = 1;
}
VOID onKeyDown(WPARAM wParam)
{
	if (1)
	{
		switch (wParam)
		{
		case VK_LEFT:
			wParam = 'J';
			break;
		case VK_RIGHT:
			wParam = 'L';
			break;
		case VK_UP:
			wParam = 'I';
			break;
		case VK_DOWN:
			wParam = 'K';
			break;
		case 'A':
			wParam = VK_LEFT;
			break;
		case 'D':
			wParam = VK_RIGHT;
			break;
		//case 'W':
		//	wParam = 'Y';
		//	break;
		//case 'S':
		//	wParam = 'N';
		case 'W':
			wParam = VK_UP;
			break;
		case 'S':
			wParam = VK_DOWN;
			break;
		//case 'Q':
		//	if (lean == 0) {
		//		lean = -scale * 4;
		//		man.rotateCamera(0, 0, lean);
		//	}
		//	break;
		//case 'E':
		//	if (lean == 0) {
		//		lean = scale * 4;
		//		man.rotateCamera(0, 0, lean);
		//	}
		//	break;
		}
	}
	Object3D * obj = cur_op;
	if (obj == NULL) {
		obj = (Object3D *)man.objs.link;
	}
	if (obj == NULL) {
		return;
	}
	switch (wParam)
	{
	case VK_CONTROL:
		is_control = 1;
		break;
	//case VK_LEFT:
	//	man.moveCamera(-1, 0, 0);
	//	break;
	//case VK_RIGHT:
	//	man.moveCamera(1, 0, 0);
	//	break;
	//case VK_UP:
	//	man.moveCamera(0, 1, 0);
	//	break;
	//case VK_DOWN:
	//	man.moveCamera(0, -1, 0);
	//	break;
	case VK_LEFT:
		if (cur_op) {
			cur_op->setUV(cur_op, cur_op->u + 1, cur_op->v);
			//cur_op->move(1, 0, 0);
		}
		break;
	case VK_RIGHT:
		if (cur_op) {
			cur_op->setUV(cur_op, cur_op->u - 1, cur_op->v);
			//cur_op->move(-1, 0, 0);
		}
		break;
	case VK_UP:
		if (cur_op) {
			cur_op->setUV(cur_op, cur_op->u, cur_op->v + 1);
			//cur_op->move(0, 1, 0);
		}
		break;
	case VK_DOWN:
		if (cur_op) {
			cur_op->setUV(cur_op, cur_op->u, cur_op->v - 1);
			//cur_op->move(0, -1, 0);
		}
		break;
	case 'Y':
		man.moveCamera(0, 0, 1);
		break;
	case 'N':
		man.moveCamera(0, 0, -1);
		break;
	case 'U':
		man.rotateCamera(0, 0, -1);
		break;
	case 'O':
		man.rotateCamera(0, 0, 1);
		break;
	case 'M':
		man.rotateCamera(1, 0, 0);
		break;
	case '/':
		man.rotateCamera(-1, 0, 0);
		break;
	case 'P':
		man.nextCamera();
		man.setCameraRange(org.x, org.y, scalex, scaley);
		break;
	case 'X':
		device.draw_line = -device.draw_line;
		break;
	case 'T':
		device.move_light = -device.move_light;
		break;
	case 'R':
		device.move_trans = -device.move_trans;
		break;
	case 'L':
		man.nextLight();
		break;
	case 'H':
		man.lgts.link->mode = -man.lgts.link->mode;
		break;
	case 'J':
		device.render_linear = -device.render_linear;
		break;
	case 'I':
		device.render_proj = -device.render_proj;
		break;
	case 'K':
		device.render_light = -device.render_light;
		break;
	case 'V':
		device.render_raytracing = -device.render_raytracing;
		break;
	case 'A':
		obj->_move(obj, 1, 0, 0);
		break;
	case 'D':
		obj->_move(obj, -1, 0, 0);
		break;
	case 'S':
		obj->_move(obj, 0, -1, 0);
		break;
	case 'W':
		obj->_move(obj, 0, 1, 0);
		break;
	case 'G':
		obj->_move(obj, 0, 0, -1);
		break;
	case 'F':
		obj->_move(obj, 0, 0, 1);
		break;
	case 'Q':
		if (cur_op) {
			Obj3D * vobj = (Obj3D*)cur_op;
			do {

				vobj->_rotate(vobj, 1, 0, 0);

				vobj = vobj->next[1];
			} while (vobj && vobj != (Obj3D*)cur_op);
		}
		break;
	case 'E':
		if (cur_op) {
			Obj3D * vobj = (Obj3D*)cur_op;
			do {

				vobj->_rotate(vobj, -1, 0, 0);

				vobj = vobj->next[1];
			} while (vobj && vobj != (Obj3D*)cur_op);
		}
		break;
	case 'Z':
		if (cur_op) {
			Obj3D * vobj = (Obj3D*)cur_op;
			do {

				vobj->_rotate(vobj, 0, 1, 0);

				vobj = vobj->next[1];
			} while (vobj && vobj != (Obj3D*)cur_op);
		}
		break;
	case 'C':
		if (cur_op) {
			Obj3D * vobj = (Obj3D*)cur_op;
			do {

				vobj->_rotate(vobj, 0, -1, 0);

				vobj = vobj->next[1];
			} while (vobj && vobj != (Obj3D*)cur_op);
		}
		break;
	case '0':
		cur_op->texture_type = 0;
		break;
	case '1':
		cur_op->texture_type = 1;
		break;
	case '2':
		cur_op->texture_type = 2;
		break;
	case '3':
		cur_op->texture_type = 3;
		break;
	case '4':
		cur_op->texture_type = 4;
		break;
	case '5':
		device.draw_oct = - device.draw_oct;
		break;
	case '6':
		device.render_thread = -device.render_thread;
		break;
	case '7':
		if (raytracing_done) {
			if (device.render_raytracing != -2) {
				device.render_raytracing = -2;
			}
			else {
				device.render_raytracing = -1;
			}
		}
		break;
	case 'B':
		DEBUG_MODE = DEBUG_MODE >> 1;
		if (DEBUG_MODE == 0)
		{
			DEBUG_MODE = 0x0B;
		}
		break;
	}

	isrefresh = 1;
}