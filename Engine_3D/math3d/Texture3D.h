// Texture3D.h
// Object for 3D (device)
//

#ifndef _Texture3D_H_
#define _Texture3D_H_

#include <string>
using namespace std;

#include "../flatform/EPaint.h"

#define MAX_TEXTURE_LINK	3
typedef struct Texture Texture;
struct Texture {
	__SUPER(MultiLinkElement, Texture, NULL);
	Texture * _prev[MAX_TEXTURE_LINK];
	Texture * _next[MAX_TEXTURE_LINK];

	/////////////////////////////////////
	INT width;
	INT height;
	DWORD *texture;
	EIMAGE image;
	void (*LoadTexture)(Texture* that, INT w, INT h, INT n);
	void (*LoadTextureEx)(Texture* that, char* filename);
	void (*destory)(Texture * that);
	/////////////////////////////////////
};
_PLATFORM void Texture_destory(Texture * that) {
	if (that->texture) {
		delete[] that->texture;
		that->texture = NULL;
	}
}
_PLATFORM Texture * _Texture(Texture * that){
	that->prev = that->_prev;
	that->next = that->_next;
	_MultiLinkElement(&that->super, MAX_TEXTURE_LINK);

	/////////////////////////////////////
	that->texture = NULL;
	that->uniqueID = 0;
	that->image = NULL;

	that->destory = Texture_destory;
	/////////////////////////////////////

	return that;
}

/////////////////////////////////////
typedef struct TexturePool TexturePool;
struct TexturePool {
	__SUPER(ElementPool, TexturePool, Texture);
};
_PLATFORM Texture * TexturePool_at(TexturePool * that, int index) {
	return &that->pool[index];
}
_PLATFORM void _TexturePool(TexturePool * that, Texture * pool, UMAP * map, int size){
	_ElementPool(&that->super, (MultiLinkElement*)pool, map, size);

	that->at = TexturePool_at;
}

#define MAX_TEXTURE	10
#define MAP_TEXTURE	GET_MAP_SIZE(MAX_TEXTURE)
typedef struct TexturePoolImp TexturePoolImp;
struct TexturePoolImp {
	Texture pool[MAX_TEXTURE];
	UMAP map[MAP_TEXTURE];

	TexturePool textPool;
};
_PLATFORM TexturePoolImp * _TexturePoolImp(TexturePoolImp *that) {

	for (int i = 0; i < MAX_TEXTURE; i++) {
		_Texture(&that->pool[i]);
	}
	_TexturePool(&that->textPool, that->pool, that->map, MAX_TEXTURE);

	return that;
}
_PLATFORM TexturePoolImp * _TexturePoolImpReload(TexturePoolImp * that) {

	for (int i = 0; i < MAX_TEXTURE; i++) {
		//keep texture and other settings
		INT width = that->pool[i].width;
		INT height = that->pool[i].height;
		INT uniqueID = that->pool[i].uniqueID;
		DWORD * texture = that->pool[i].texture;
		_Texture(&that->pool[i]);
		//restore
		that->pool[i].width = width;
		that->pool[i].height = height;
		that->pool[i].texture = texture;
		that->pool[i].uniqueID = uniqueID;
	}
	_TexturePool(&that->textPool, that->pool, that->map, MAX_TEXTURE);

	return that;
}
typedef struct TextureMan TextureMan;
struct TextureMan{
	__SUPER(MultiLinkBase, TextureMan, Texture);

	TexturePool * textPool;
	
	////////////////////////////
	void (*clearLink)(TextureMan * that);
	////////////////////////////
};
_PLATFORM void TextureMan_clearLink(TextureMan * that){
	if (that->link) {
		Texture * temp = that->link;
		do {
			if (that->removeLink(that, temp) == NULL) {
				break;
			}
			if (!temp->free(temp)) {
				temp->destory(temp);
				that->textPool->back(that->textPool, temp);
			}

			temp = that->link;
		} while (temp);
	}
}
_PLATFORM TextureMan * _TextureMan(TextureMan * that, int index, TexturePoolImp * poolImp){
	_MultiLinkBase(&that->super, index);

	that->textPool = &poolImp->textPool;

	///////////////////////////////////////
	that->clearLink = TextureMan_clearLink;
	///////////////////////////////////////

	return that;
}
/////////////////////////////////////


typedef class TextureManager TextureManager;
class TextureManager {
public:
	_PLATFORM TextureManager(){
	}
	_PLATFORM ~TextureManager() {
		textures.clearLink(&textures);
	}
	_PLATFORM void Init() {
		_TextureMan(&textures, 0, texturePoolImp);
	}

	_PLATFORM void Reload() {	
		//The pool was copied from host, and the links are pointing to host objects
		//so we need to remove these links and reload them to link
		//and reload all the host functions to device function
		_TexturePoolImpReload(this->texturePoolImp);
		for (int i = 0; i < MAX_TEXTURE; i++) {
			Texture& t = this->texturePoolImp->pool[i];
			if (t.texture) {
				//t.uniqueID = i;
				for (int j = 0; j < MAX_TEXTURE_LINK; j ++) {
					t.prev[j] = NULL;
					t.next[j] = NULL;
				}
				this->textures.insertLink(&this->textures, &t, NULL, NULL);
			}
		}
	}

	TextureMan textures;
	TexturePoolImp * texturePoolImp;

};


#endif