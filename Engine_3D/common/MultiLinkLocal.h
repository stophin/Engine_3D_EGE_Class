// MultiLinkLocal.h
//

#ifndef _MULTILINKLOCAL_
#define _MULTILINKLOCAL_
#define NULL	0

#include "../common/EPoint.h"

//////////////////////////////////////////////////////////
#define __SUPERLOCAL(B, T, E) \
	union {\
		B super; \
		struct {\
			Template##B (T, E)\
		}; \
	}
//////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
typedef struct MultiLinkLocalElement {
#define MultiLinkLocalElementTemplate(T)\
	int linkcount;\
	int uniqueID;\
	T ** prev;\
	T ** next;\
	void(*_final)(T *that);\
	T * (*free)(T * that);\
	void(*clear)(T * that);
#define TemplateMultiLinkLocalElement(T, E) MultiLinkLocalElementTemplate(struct T)
	TemplateMultiLinkLocalElement(MultiLinkLocalElement, NULL)
}MultiLinkLocalElement;
 void MultiLinkLocalElement_clear(MultiLinkLocalElement * that);
 MultiLinkLocalElement * MultiLinkLocalElement_free(MultiLinkLocalElement * that);
 void _MultiLinkLocalElement(MultiLinkLocalElement * that, int linkcount);
///////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
typedef struct MultiLinkLocalBase {
#define MultiLinkLocalBaseTemplate(T, E) \
	int linkcount;\
	int linkindex;\
	E * link;\
	E * (*getLink)(T * that, int uniqueID); \
	E * (*insertLink)(T * that, E * link, E * before, E * after); \
	E * (*removeLink)(T * that, E * link); \
	E * (*get)(T * that, int index); \
	E * (*prev)(T *that, E * link); \
	E * (*next)(T *that, E * link);
#define TemplateMultiLinkLocalBase(T, E) MultiLinkLocalBaseTemplate(struct T, struct E)
	TemplateMultiLinkLocalBase(MultiLinkLocalBase, MultiLinkLocalElement)
}MultiLinkLocalBase;
 MultiLinkLocalElement * MultiLinkLocalBase_removeLink(MultiLinkLocalBase * that, MultiLinkLocalElement * link);
 MultiLinkLocalElement * MultiLinkLocalBase_get(MultiLinkLocalBase * that, int index);
 MultiLinkLocalElement * MultiLinkLocalBase_getLink(MultiLinkLocalBase * that, int uniqueID);
 MultiLinkLocalElement * MultiLinkLocalBase_insertLink(MultiLinkLocalBase * that, MultiLinkLocalElement * link, MultiLinkLocalElement * before, MultiLinkLocalElement * after);
 MultiLinkLocalElement * MultiLinkLocalBase_prev(MultiLinkLocalBase *that, MultiLinkLocalElement * link);
 MultiLinkLocalElement * MultiLinkLocalBase_next(MultiLinkLocalBase *that, MultiLinkLocalElement * link);
 void _MultiLinkLocalBase(MultiLinkLocalBase * that, int linkindex);
////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
typedef unsigned char UMAP;
#define MAP_SHIFT	8
#define MAP_MASK	0xFF
#define POOL_MAX	10
#define GET_MAP_SIZE(x) (x / MAP_SHIFT + 1)
#define MAP_MAX	GET_MAP_SIZE(POOL_MAX)

typedef struct ElementLocalPool {
#define ElementLocalPoolTemplate(T, E)\
	E * pool;\
	UMAP * map;\
	int size;\
	int msize;\
	int count;\
	E * (*at)(T * that, int index);\
	E * (*get)(T * that);\
	void(*back)(T * that, E * o);
#define TemplateElementLocalPool(T, E) ElementLocalPoolTemplate(struct T, struct E)
	TemplateElementLocalPool(ElementLocalPool, MultiLinkLocalElement)
}ElementLocalPool;
 MultiLinkLocalElement * ElementLocalPool_at(ElementLocalPool * that, int index);
 MultiLinkLocalElement * ElementLocalPool_get(ElementLocalPool * that);
 void ElementLocalPool_back(ElementLocalPool * that, MultiLinkLocalElement * o);
 void _ElementLocalPool(ElementLocalPool * that, MultiLinkLocalElement * pool, UMAP * map, int size);
///////////////////////////////////////////////////////////



#endif
