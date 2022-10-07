// MultiLink.h
//

#ifndef _MULTILINK_
#define _MULTILINK_
#define NULL	0

#include "../common/EPoint.h"

//////////////////////////////////////////////////////////
#define __SUPER(B, T, E) \
	union {\
		B super; \
		struct {\
			Template##B (T, E)\
		}; \
	}
//////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
typedef struct MultiLinkElement {
#define MultiLinkElementTemplate(T)\
	int linkcount;\
	int linkindex;\
	int uniqueID;\
	T ** prev;\
	T ** next;\
	void(*_final)(T *that);\
	T * (*free)(T * that);\
	void(*clear)(T * that);
#define TemplateMultiLinkElement(T, E) MultiLinkElementTemplate(struct T)
	TemplateMultiLinkElement(MultiLinkElement, NULL)
}MultiLinkElement;
_PLATFORM void MultiLinkElement_clear(MultiLinkElement * that);
_PLATFORM MultiLinkElement * MultiLinkElement_free(MultiLinkElement * that);
_PLATFORM void _MultiLinkElement(MultiLinkElement * that, int linkcount);
///////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
typedef struct MultiLinkBase {
#define MultiLinkBaseTemplate(T, E) \
	int linkcount;\
	int linkindex;\
	E * link;\
	E * (*getLink)(T * that, int uniqueID); \
	E * (*insertLink)(T * that, E * link, E * before, E * after); \
	E * (*removeLink)(T * that, E * link); \
	E * (*get)(T * that, int index); \
	E * (*prev)(T *that, E * link); \
	E * (*next)(T *that, E * link);
#define TemplateMultiLinkBase(T, E) MultiLinkBaseTemplate(struct T, struct E)
	TemplateMultiLinkBase(MultiLinkBase, MultiLinkElement)
}MultiLinkBase;
_PLATFORM MultiLinkElement * MultiLinkBase_removeLink(MultiLinkBase * that, MultiLinkElement * link);
_PLATFORM MultiLinkElement * MultiLinkBase_get(MultiLinkBase * that, int index);
_PLATFORM MultiLinkElement * MultiLinkBase_getLink(MultiLinkBase * that, int uniqueID);
_PLATFORM MultiLinkElement * MultiLinkBase_insertLink(MultiLinkBase * that, MultiLinkElement * link, MultiLinkElement * before, MultiLinkElement * after);
_PLATFORM MultiLinkElement * MultiLinkBase_prev(MultiLinkBase *that, MultiLinkElement * link);
_PLATFORM MultiLinkElement * MultiLinkBase_next(MultiLinkBase *that, MultiLinkElement * link);
_PLATFORM void _MultiLinkBase(MultiLinkBase * that, int linkindex);
////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
typedef unsigned char UMAP;
#define MAP_SHIFT	8
#define MAP_MASK	0xFF
#define POOL_MAX	1000
#define GET_MAP_SIZE(x) (x / MAP_SHIFT + 1)
#define MAP_MAX	GET_MAP_SIZE(POOL_MAX)

typedef struct ElementPool {
#define ElementPoolTemplate(T, E)\
	E * pool;\
	UMAP * map;\
	int size;\
	int msize;\
	int count;\
	E * (*at)(T * that, int index);\
	E * (*get)(T * that);\
	void(*back)(T * that, E * o);
#define TemplateElementPool(T, E) ElementPoolTemplate(struct T, struct E)
	TemplateElementPool(ElementPool, MultiLinkElement)
}ElementPool;
_PLATFORM MultiLinkElement * ElementPool_at(ElementPool * that, int index);
_PLATFORM MultiLinkElement * ElementPool_get(ElementPool * that);
_PLATFORM void ElementPool_back(ElementPool * that, MultiLinkElement * o);
_PLATFORM void _ElementPool(ElementPool * that, MultiLinkElement * pool, UMAP * map, int size);
///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
_PLATFORM void MultiLinkElement_clear(MultiLinkElement * that) {
	int i;
	for (i = 0; i < that->linkcount; i++) {
		that->prev[i] = NULL;
		that->next[i] = NULL;
	}
}
_PLATFORM MultiLinkElement * MultiLinkElement_free(MultiLinkElement * that) {
	int i;
	for (i = 0; i < that->linkcount; i++) {
		if (that->prev[i] != NULL || that->next[i] != NULL) {
			return that;
		}
	}
	return NULL;
}
_PLATFORM void _MultiLinkElement(MultiLinkElement * that, int linkcount) {
	that->linkcount = linkcount;
	that->linkindex = -1;

	that->clear = MultiLinkElement_clear;
	that->free = MultiLinkElement_free;
	that->_final = NULL;

	that->clear(that);
}
_PLATFORM void _MultiLinkElementEx(MultiLinkElement * that, int linkcount) {
	that->linkcount = linkcount;

	that->clear = MultiLinkElement_clear;
	that->free = MultiLinkElement_free;
	that->_final = NULL;

	that->clear(that);
}
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
_PLATFORM MultiLinkElement * MultiLinkBase_removeLink(MultiLinkBase * that, MultiLinkElement * link) {

	MultiLinkElement * before, *after;
	if (link == NULL)
	{
		return NULL;
	}
	if (that->linkindex < 0)
	{
		return NULL;
	}
	if (link->prev[that->linkindex] == NULL || link->next[that->linkindex] == NULL)
	{
		return NULL;
	}
	before = link->prev[that->linkindex];
	after = link->next[that->linkindex];

	if (!before || !after) {
		return NULL;
	}
	before->next[that->linkindex] = after;
	after->prev[that->linkindex] = before;
	link->prev[that->linkindex] = NULL;
	link->next[that->linkindex] = NULL;

	if (that->link == link)
	{
		that->link = after;
	}
	if (that->link == link)
	{
		that->link = NULL;
	}

	that->linkcount = that->linkcount - 1;

	return link;
}
_PLATFORM MultiLinkElement * MultiLinkBase_getLink(MultiLinkBase * that, int uniqueID) {
	if (that->link == NULL) {
		return NULL;
	}
	MultiLinkElement * temp = that->link;
	do {
		if (temp->uniqueID == uniqueID) {
			return temp;
		}
		temp = temp->next[that->linkindex];
	} while (temp && temp != that->link);
	return NULL;
}
_PLATFORM MultiLinkElement * MultiLinkBase_get(MultiLinkBase * that, int index) {
	MultiLinkElement * temp;
	if (that->link == NULL)
	{
		return NULL;
	}
	temp = that->link;
	do
	{
		temp = temp->next[that->linkindex];
	} while (temp && temp != that->link && --index);
	return temp;
}

_PLATFORM MultiLinkElement * MultiLinkBase_insertLink(MultiLinkBase * that, MultiLinkElement * link, MultiLinkElement * before, MultiLinkElement * after) {
	MultiLinkElement * _link;
	if (link == NULL)
	{
		//	return link;
	}
	if (that->link == NULL)
	{
		that->link = link;

		that->link->prev[that->linkindex] = link;
		that->link->next[that->linkindex] = link;

		that->linkcount = that->linkcount + 1;

		return link;
	}
	else
	{
		_link = NULL;
		if (before == that->link)
		{
			_link = link;
		}
		if (before == NULL && after == NULL)
		{
			before = that->link;
			after = that->link->prev[that->linkindex];
		}
		else if (before == NULL)
		{
			before = after->next[that->linkindex];
		}
		else if (after == NULL)
		{
			after = before->prev[that->linkindex];
		}
		else /* before != NULL && after != NULL*/
		{
			if (before->prev[that->linkindex] != after || after->next[that->linkindex] != before)
			{
				return link;
			}
		}
		if (before == NULL || after == NULL ||
			before->prev[that->linkindex] == NULL ||
			after->next[that->linkindex] == NULL)
		{
			return link;
		}

		if (link) {
			link->prev[that->linkindex] = after;
			link->next[that->linkindex] = before;
		}
		after->next[that->linkindex] = link;
		before->prev[that->linkindex] = link;

		if (_link)
		{
			that->link = _link;
		}

		that->linkcount = that->linkcount + 1;
	}
	return link;
}
_PLATFORM MultiLinkElement * MultiLinkBase_prev(MultiLinkBase *that, MultiLinkElement * link) {
	if (link == NULL)
	{
		return NULL;
	}
	return link->prev[that->linkindex];
}
_PLATFORM MultiLinkElement * MultiLinkBase_next(MultiLinkBase *that, MultiLinkElement * link) {
	if (link == NULL)
	{
		return NULL;
	}
	return link->next[that->linkindex];
}
_PLATFORM void _MultiLinkBase(MultiLinkBase * that, int linkindex) {
	that->linkcount = 0;
	that->linkindex = linkindex;
	that->link = NULL;

	that->insertLink = MultiLinkBase_insertLink;
	that->prev = MultiLinkBase_prev;
	that->next = MultiLinkBase_next;
	that->removeLink = MultiLinkBase_removeLink;
	that->get = MultiLinkBase_get;
	that->getLink = MultiLinkBase_getLink;
}
_PLATFORM void _MultiLinkBaseEx(MultiLinkBase * that, int linkindex) {
	that->linkcount = 0;
	that->linkindex = linkindex;
	that->link = NULL;

	that->insertLink = MultiLinkBase_insertLink;
	that->prev = MultiLinkBase_prev;
	that->next = MultiLinkBase_next;
	that->removeLink = MultiLinkBase_removeLink;
	that->get = MultiLinkBase_get;
	that->getLink = MultiLinkBase_getLink;
}
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
_PLATFORM MultiLinkElement * ElementPool_at(ElementPool * that, int index) {
	// Inherit struct must override this function
	// because the size of the type of pool is different
	// Note: in this kind of inherit, be careful when
	// using arry of specified type, but there's no
	// need to worry about using pointer of the type
	// e.g. MultiLinkElement has pointer array :
	// prev and next, and there's no need to
	// override any get/set function in inherit struct
	return &that->pool[index];
}
_PLATFORM MultiLinkElement * ElementPool_get(ElementPool * that) {
	int i, j, index;
	for (i = 0, index = 0; i < that->msize && index < that->size; i++, index += MAP_SHIFT) {
		if (that->map[i] & MAP_MASK) {
			for (j = 0; j < MAP_SHIFT && index < that->size; j++, index++) {
				if (that->map[i] & (0x01 << j)) {
					that->map[i] &= ~(0x01 << j);
					MultiLinkElement * ret = that->at(that, index);
					ret->linkindex = index;
					return ret;
				}
			}
		}
	}
	return NULL;
}
_PLATFORM void ElementPool_back(ElementPool * that, MultiLinkElement * o) {
	int i, j, index;
	if (o == NULL) {
		return;
	}
	if (o->linkindex >= 0 && o->linkindex < that->size) {
		index = o->linkindex;
		i = index / MAP_SHIFT;
		j = index - i * MAP_SHIFT;
		that->map[i] |= (0x01 << j);
		return;
	}
	for (index = 0; index < that->size; index++) {
		if (that->at(that, index) == o) {
			i = index / MAP_SHIFT;
			j = index - i * MAP_SHIFT;
			that->map[i] |= (0x01 << j);
			return;
		}
	}
}
_PLATFORM void _ElementPool(ElementPool * that, MultiLinkElement * pool, UMAP * map, int size) {
	int i;
	if (size > POOL_MAX) {
		//size = POOL_MAX;
	}
	that->pool = pool;
	that->map = map;
	that->size = size;

	that->at = ElementPool_at;
	that->get = ElementPool_get;
	that->back = ElementPool_back;

	that->msize = size / MAP_SHIFT + 1;
	if (that->msize > MAP_MAX) {
		//that->msize = MAP_MAX;
	}

	for (i = 0; i < that->msize; i++) {
		that->map[i] = MAP_MASK;
	}
}
_PLATFORM void _ElementPoolEx(ElementPool * that, MultiLinkElement * pool, UMAP * map, int size) {
	int i;
	if (size > POOL_MAX) {
		//size = POOL_MAX;
	}
	that->pool = pool;
	that->map = map;
	that->size = size;

	that->at = ElementPool_at;
	that->get = ElementPool_get;
	that->back = ElementPool_back;

	that->msize = size / MAP_SHIFT + 1;
	if (that->msize > MAP_MAX) {
		//that->msize = MAP_MAX;
	}

	for (i = 0; i < that->msize; i++) {
		that->map[i] = MAP_MASK;
	}
}
///////////////////////////////////////////////////////////


#endif

