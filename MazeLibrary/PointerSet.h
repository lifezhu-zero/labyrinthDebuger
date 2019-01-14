#pragma once

#ifndef __POINTER_SET__
#define __POINTER_SET__

//�԰����ɱ䳤������/ָ����������в���

#ifdef __cplusplus
extern "C" {
#endif
	void InitializePS(void*** _array, int* _length);
	void AllocPS(void*** _array, int* _length, int _len);
	void AddAPointerPS(void*** _array, int* _length, void* _insert);
	void RemoveAPointerPS(void*** _array, int* _length, void* _erase);
	void** GetPointerPS(void** _array, int _length,int _pos);
#ifdef __cplusplus
}
#endif

#endif