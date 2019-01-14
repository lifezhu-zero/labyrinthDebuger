#include <memory.h>
#include <malloc.h>

#include "PointerSet.h"

void InitializePS(void*** _array, int* _length)
{
	if (_array != NULL && _length != NULL)
	{
		*_array = NULL;
		_length = 0;
	}
}

void AllocPS(void*** _array, int* _length, int _len)
{
	if (_array != NULL && _length != NULL)
	{
		if(*_array!=NULL)
			if (**_array != NULL)
				free(**_array);
		*_array = malloc(_len * sizeof(void*));
		*_length = _len;
		memset(*_array, 0, _len * sizeof(void*));
	}
}

void AddAPointerPS(void*** _array, int* _length, void* _insert)
{
	if (_array != NULL && _length != NULL)
	{
		int size = *_length ;
		void** buffer = malloc(sizeof(void*)*(size+1));
		memcpy(buffer, *_array, sizeof(void*)*size);
		buffer[size] = _insert;
		*_length = 0;
		free(*_array);
		*_array = buffer;
		*_length = size + 1;
	}
}

void RemoveAPointerPS(void*** _array, int* _length, void* _erase)
{
	if (_array != NULL && _length != NULL)
	{
		int size = *_length;
		int pos;
		for (pos = 0; pos < size; pos++)
		{
			if (*_array[pos] == _erase)
				break;
		}
		if (pos < size)
		{
			void** buffer = malloc(sizeof(void*)*(size - 1));
			memcpy(buffer, *_array, sizeof(void*)*pos);
			memcpy(buffer + sizeof(void*)*pos, *_array + pos + 1, sizeof(void*)*(size - pos - 1));
			*_length = 0;
			free(*_array);
			*_array = buffer;
			*_length = size - 1;
		}
	}
}

void** GetPointerPS(void** _array, int _length, int _pos)
{
	if (_length > _pos && _array!=NULL)
	{
		return &_array[_pos];
	}
	return NULL;
}