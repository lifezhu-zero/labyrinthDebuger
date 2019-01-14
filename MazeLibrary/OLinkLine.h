#ifndef __OLINKLINE_H_
#define __OLINKLINE_H_

#ifdef __cplusplus
extern "C"{
#endif
	//环状链表队列

	typedef struct tagLinkLine//链表单元
	{
		void* item_;//用户单元
		struct tagLinkLine* next_;//下一单元
		struct tagLinkLine* prev_;//前一单元
	}LinkLine, *LinkLineP, ** LinkLinePP;

	typedef struct tagLinkLineControl//链表控制结构
	{
		struct tagLinkLine* Header_;//作为队列的第一个单元
	}LinkLineControl,*LinkLineControlP;

	LinkLineControlP OLL_Alloc();
	void OLL_Release(LinkLineControlP);
	void OLL_Push(LinkLineControlP, void*);
	void* OLL_Pop(LinkLineControlP);
	int OLL_Size(LinkLineControlP);
#ifdef __cplusplus
	};
#endif

#endif
	                                               