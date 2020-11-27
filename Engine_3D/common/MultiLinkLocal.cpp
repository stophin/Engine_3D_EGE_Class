#include "MultiLinkLocal.h"


///////////////////////////////////////////////////////////
void MultiLinkLocalElement_clear(MultiLinkLocalElement * that) {
	int i;
	for (i = 0; i < that->linkcount; i++) {
		that->prev[i] = NULL;
		that->next[i] = NULL;
	}
}
MultiLinkLocalElement * MultiLinkLocalElement_free(MultiLinkLocalElement * that) {
	int i;
	for (i = 0; i < that->linkcount; i++) {
		if (that->prev[i] != NULL || that->next[i] != NULL) {
			return that;
		}
	}
	return NULL;
}
void _MultiLinkLocalElement(MultiLinkLocalElement * that, int linkcount) {
	that->linkcount = linkcount;

	that->clear = MultiLinkLocalElement_clear;
	that->free = MultiLinkLocalElement_free;
	that->_final = NULL;

	that->clear(that);
}
void _MultiLinkLocalElementEx(MultiLinkLocalElement * that, int linkcount) {
	that->linkcount = linkcount;

	that->clear = MultiLinkLocalElement_clear;
	that->free = MultiLinkLocalElement_free;
	that->_final = NULL;

	that->clear(that);
}
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
MultiLinkLocalElement * MultiLinkLocalBase_removeLink(MultiLinkLocalBase * that, MultiLinkLocalElement * link) {

	MultiLinkLocalElement * before, *after;
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
MultiLinkLocalElement * MultiLinkLocalBase_getLink(MultiLinkLocalBase * that, int uniqueID) {
	if (that->link == NULL) {
		return NULL;
	}
	MultiLinkLocalElement * temp = that->link;
	do {
		if (temp->uniqueID == uniqueID) {
			return temp;
		}
		temp = temp->next[that->linkindex];
	} while (temp && temp != that->link);
	return NULL;
}
MultiLinkLocalElement * MultiLinkLocalBase_get(MultiLinkLocalBase * that, int index) {
	MultiLinkLocalElement * temp;
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

MultiLinkLocalElement * MultiLinkLocalBase_insertLink(MultiLinkLocalBase * that, MultiLinkLocalElement * link, MultiLinkLocalElement * before, MultiLinkLocalElement * after) {
	MultiLinkLocalElement * _link;
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
MultiLinkLocalElement * MultiLinkLocalBase_prev(MultiLinkLocalBase *that, MultiLinkLocalElement * link) {
	if (link == NULL)
	{
		return NULL;
	}
	return link->prev[that->linkindex];
}
MultiLinkLocalElement * MultiLinkLocalBase_next(MultiLinkLocalBase *that, MultiLinkLocalElement * link) {
	if (link == NULL)
	{
		return NULL;
	}
	return link->next[that->linkindex];
}
void _MultiLinkLocalBase(MultiLinkLocalBase * that, int linkindex) {
	that->linkcount = 0;
	that->linkindex = linkindex;
	that->link = NULL;

	that->insertLink = MultiLinkLocalBase_insertLink;
	that->prev = MultiLinkLocalBase_prev;
	that->next = MultiLinkLocalBase_next;
	that->removeLink = MultiLinkLocalBase_removeLink;
	that->get = MultiLinkLocalBase_get;
	that->getLink = MultiLinkLocalBase_getLink;
}
void _MultiLinkLocalBaseEx(MultiLinkLocalBase * that, int linkindex) {
	that->linkcount = 0;
	that->linkindex = linkindex;
	that->link = NULL;

	that->insertLink = MultiLinkLocalBase_insertLink;
	that->prev = MultiLinkLocalBase_prev;
	that->next = MultiLinkLocalBase_next;
	that->removeLink = MultiLinkLocalBase_removeLink;
	that->get = MultiLinkLocalBase_get;
	that->getLink = MultiLinkLocalBase_getLink;
}
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
MultiLinkLocalElement * ElementLocalPool_at(ElementLocalPool * that, int index) {
	// Inherit struct must override this function
	// because the size of the type of pool is different
	// Note: in this kind of inherit, be careful when
	// using arry of specified type, but there's no
	// need to worry about using pointer of the type
	// e.g. MultiLinkLocalElement has pointer array :
	// prev and next, and there's no need to
	// override any get/set function in inherit struct
	return &that->pool[index];
}
MultiLinkLocalElement * ElementLocalPool_get(ElementLocalPool * that) {
	int i, j, index;
	for (i = 0, index = 0; i < that->msize && index < that->size; i++, index += MAP_SHIFT) {
		if (that->map[i] & MAP_MASK) {
			for (j = 0; j < MAP_SHIFT && index < that->size; j++, index++) {
				if (that->map[i] & (0x01 << j)) {
					that->map[i] &= ~(0x01 << j);
					MultiLinkLocalElement * ret = that->at(that, index);
					ret->linkindex = index;
					return ret;
				}
			}
		}
	}
	return NULL;
}
void ElementLocalPool_back(ElementLocalPool * that, MultiLinkLocalElement * o) {
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
void _ElementLocalPool(ElementLocalPool * that, MultiLinkLocalElement * pool, UMAP * map, int size) {
	int i;
	if (size > POOL_MAX) {
		//size = POOL_MAX;
	}
	that->pool = pool;
	that->map = map;
	that->size = size;

	that->at = ElementLocalPool_at;
	that->get = ElementLocalPool_get;
	that->back = ElementLocalPool_back;

	that->msize = size / MAP_SHIFT + 1;
	if (that->msize > MAP_MAX) {
		//that->msize = MAP_MAX;
	}

	for (i = 0; i < that->msize; i++) {
		that->map[i] = MAP_MASK;
	}
}
void _ElementLocalPoolEx(ElementLocalPool * that, MultiLinkLocalElement * pool, UMAP * map, int size) {
	int i;
	if (size > POOL_MAX) {
		//size = POOL_MAX;
	}
	that->pool = pool;
	that->map = map;
	that->size = size;

	that->at = ElementLocalPool_at;
	that->get = ElementLocalPool_get;
	that->back = ElementLocalPool_back;

	that->msize = size / MAP_SHIFT + 1;
	if (that->msize > MAP_MAX) {
		//that->msize = MAP_MAX;
	}

	for (i = 0; i < that->msize; i++) {
		that->map[i] = MAP_MASK;
	}
}
///////////////////////////////////////////////////////////
