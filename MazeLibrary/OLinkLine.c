#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include "OLinkLine.h"

static LinkLineP OLL_AllocNode(void* _item);
static void OLL_ReleaseNode(LinkLineP _node);

LinkLineControlP OLL_Alloc()
{
	LinkLineControlP ret = (LinkLineControlP)malloc(sizeof(LinkLineControl));
	ret->Header_ = NULL;
	return ret;
}

void OLL_Release(LinkLineControlP _ll)
{
	if (_ll != NULL)
	{
		LinkLineP current = _ll->Header_;
		while (current != NULL && current->next_ != _ll->Header_)
		{
			LinkLineP rmv = current;
			free(rmv);
			current = current->next_;
		}
		if (current != NULL)
			free(current);
		free(_ll);
	}
}

void OLL_Push(LinkLineControlP _ll, void* _item)
{
	if (_ll != NULL)
	{
		if (_ll->Header_ != NULL)
		{
			LinkLineP first = _ll->Header_, newNode;
			assert(first->next_ != NULL);
			assert(first->next_ != NULL);

			newNode = OLL_AllocNode(_item);
			newNode->next_ = first;
			newNode->prev_ = first->prev_;
			first->prev_ = newNode;
			newNode->prev_->next_ = newNode;
			_ll->Header_ = newNode;
		}
		else
		{
			LinkLineP newNode;
			newNode = OLL_AllocNode(_item);
			newNode->next_ = newNode;
			newNode->prev_ = newNode;
			_ll->Header_ = newNode;
		}
	}
}

void* OLL_Pop(LinkLineControlP _ll)
{
	if (_ll != NULL)
	{
		if (_ll->Header_ != NULL)
		{
			LinkLineP ret = _ll->Header_;
			void* retItem = _ll->Header_->item_;

			assert((ret->next_ != NULL
				&& ret->prev_ != NULL)
				|| (ret->next_ == ret && ret->prev_ == ret)
			);

			if (ret->next_ != ret)
			{
				LinkLineP prev = ret->prev_,next=ret->next_;
				next->prev_ = prev;
				prev->next_ = next;
				_ll->Header_ = next;				
			}
			else
			{
				_ll->Header_ = NULL;
			}
			
			free(ret);
			return retItem;
		}
	}
	return NULL;
}

int OLL_Size(LinkLineControlP _ll)
{
	if (_ll == NULL || _ll->Header_ == NULL)
		return 0;
	
	LinkLineP p = _ll->Header_;
	int ret = 0;

	while (p->next_ != p)
	{
		p = p->next_;
		ret++;
	}
	return ret;
}

LinkLineP OLL_AllocNode(void* _item)
{
	LinkLineP ret = (LinkLineP)malloc(sizeof(LinkLine));
	ret->item_ = _item;
	ret->next_ = NULL;
	ret->prev_ = NULL;
	return ret;
}

void OLL_ReleaseNode(LinkLineP _node)
{
	free(_node);
}
