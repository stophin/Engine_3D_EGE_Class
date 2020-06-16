// Texture.h
// Object for 3D (local)
//

#ifndef _Texture_H_
#define _Texture_H_

#include "../common/MultiLinkLocal.h"

#include <string>
using namespace std;

#include "../flatform/EPaint.h"

#define MAX_TEXTURELOCAL_LINK	3
typedef struct TextureLocal TextureLocal;
struct TextureLocal {
	__SUPERLOCAL(MultiLinkLocalElement, TextureLocal, NULL);
	TextureLocal * _prev[MAX_TEXTURELOCAL_LINK];
	TextureLocal * _next[MAX_TEXTURELOCAL_LINK];

	/////////////////////////////////////
	INT width;
	INT height;
	DWORD *texture;
	EIMAGE image;
	void (*LoadTexture)(TextureLocal * that, INT w, INT h, INT n);
	void (*LoadTextureEx)(TextureLocal * that, char *filename);
	void (*destory)(TextureLocal * that);
	/////////////////////////////////////
};
void TextureLocal_destory(TextureLocal * that);
void TextureLocal_LoadTextureEx(TextureLocal* that, char *filename);
void TextureLocal_LoadTexture(TextureLocal* that, INT w, INT h, INT n);
TextureLocal * _TextureLocal(TextureLocal * that);

/////////////////////////////////////
typedef struct TextureLocalPool TextureLocalPool;
struct TextureLocalPool {
	__SUPERLOCAL(ElementLocalPool, TextureLocalPool, TextureLocal);
};
TextureLocal * TextureLocalPool_at(TextureLocalPool * that, int index);
void _TextureLocalPool(TextureLocalPool * that, TextureLocal * pool, UMAP * map, int size);

#define MAX_TEXTURELOCAL	10
#define MAP_TEXTURELOCAL	GET_MAP_SIZE(MAX_TEXTURELOCAL)
typedef struct TextureLocalPoolImp TextureLocalPoolImp;
struct TextureLocalPoolImp {
	TextureLocal pool[MAX_TEXTURELOCAL];
	UMAP map[MAP_TEXTURELOCAL];

	TextureLocalPool textPool;
};
TextureLocalPoolImp * _TextureLocalPoolImp(TextureLocalPoolImp *that);
typedef struct TextureLocalMan TextureLocalMan;
struct TextureLocalMan{
	__SUPERLOCAL(MultiLinkLocalBase, TextureLocalMan, TextureLocal);

	TextureLocalPool * textPool;
	
	////////////////////////////
	void (*clearLink)(TextureLocalMan * that);
	////////////////////////////
};
void TextureLocalMan_clearlink(TextureLocalMan * that);
TextureLocalMan * _TextureLocalMan(TextureLocalMan * that, int index, TextureLocalPoolImp * poolImp);
/////////////////////////////////////


typedef class TextureLocalManager TextureLocalManager;
class TextureLocalManager {
public:
	TextureLocalManager(){
	}
	~TextureLocalManager() {
		textures.clearLink(&textures);
	}
	void Init() {
		_TextureLocalMan(&textures, 0, texturePoolImp);
	}

	TextureLocalMan textures;
	TextureLocalPoolImp * texturePoolImp;

	INT addTexture(char * filename) {
		TextureLocal * texture = texturePoolImp->textPool.get(&texturePoolImp->textPool);
		if (!texture) {
			return 0;
		}
		_TextureLocal(texture);
		texture->uniqueID = this->textures.linkcount;
		texture->LoadTextureEx(texture, filename);

		this->textures.insertLink(&this->textures, texture, NULL, NULL);

		return texture->uniqueID;
	}

	INT addTexture(INT w, INT h, INT n) {
		TextureLocal * texture = texturePoolImp->textPool.get(&texturePoolImp->textPool);
		if (!texture) {
			return 0;
		}
		_TextureLocal(texture);
		texture->uniqueID = this->textures.linkcount;
		texture->LoadTexture(texture, w, h, n);

		this->textures.insertLink(&this->textures, texture, NULL, NULL);

		return texture->uniqueID;
	}

};



#endif