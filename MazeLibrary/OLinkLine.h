#ifndef __OLINKLINE_H_
#define __OLINKLINE_H_

#ifdef __cplusplus
extern "C"{
#endif
	//��״�������

	typedef struct tagLinkLine//����Ԫ
	{
		void* item_;//�û���Ԫ
		struct tagLinkLine* next_;//��һ��Ԫ
		struct tagLinkLine* prev_;//ǰһ��Ԫ
	}LinkLine, *LinkLineP, ** LinkLinePP;

	typedef struct tagLinkLineControl//������ƽṹ
	{
		struct tagLinkLine* Header_;//��Ϊ���еĵ�һ����Ԫ
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
	                                               