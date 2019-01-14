#include <stdlib.h>
#include <memory.h>
#include <malloc.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "Maze.h"
#include "PointerSet.h"

//�ݹ��������ʼ���Թ������з���
//_DimNum ά�ȵ���Ŀ����_Dim����ĳ���
//_Dim ��ά�ȵĳ߶�
//_currentDimPos ��ǰ������ά�����ڵ�ά��ֵ
//_currentDim ��ǰ����������
//_maze ���淿�䡢�ŵĽṹ
//_currentDoor ��ǰʹ�õ���ָ��
//_currentRoom ��ǰ����ķ���
static void ThroughAndInitializeAllRoomInMaze(
	DIMLENGTH _DimNum,
	DIMCONTENT _Dim,
	DIMINDEX _currentDimPos,
	DIMCONTENT _CurrentDim,
	PMaze _maze,
	PPDoor* _currentDoor,
	PPRoom _currentRoom);
//Ϊ����������е���
// _maze �Թ�
// _room ��ǰ���������ڵķ���
// _currentDoor ��ǰ�Ѿ�����ʹ�õ��ŵ�λ��
// ������ά�ռ��Լ����ϵĸ����Դ�����
// �����ƶ�ÿ����һ��ά�ȣ�һ��������ķ���ֻ�࿪������
// �����ŵĴ���Ϊ�����¡������ҡ�ǰ��
static void AllocDoorForRoom(PMaze _maze, PRoom _room, PPDoor* _currentDoor);
static int FindRoomFirstUnUsedDoor(PRoom _room, int _size);

NONE InitializeDoor(PPDoor _door)
{
	*_door = (PDoor)malloc(sizeof(Door));
	memset(*_door, 0, sizeof(Door));
	(*_door)->flags_ = 0x0;
	(*_door)->isClosed_ = 0;
	(*_door)->linkedRooms_[0] = NULL;
	(*_door)->linkedRooms_[1] = NULL;
}

//�������Ƿ�����
BOOL isDoorCreated(PDoor _door)
{
	return (_door->flags_ & 0x40000000) != 0;
}

NONE AllocMaze(DIMLENGTH _dimNum, DIMCONTENT _dimIn, PMaze _outMaze)
{
	//assert(_dimIn != NULL && _dimNum > 0);
	//����Ҫ����ά��
	DIMCONTENT dimRegular;
	int i, j, factorEveryDim;
	int totalRoom;//��������
	int totalWall;//ǽ��/������
	int totalOutterWall;//��ǽ����
	DIMCONTENT currentDim;//���Գ�ʼ�������ά������
	PRoom currentRoom;
	Door **currentDoor;
	int size;

	if (_outMaze == NULL)
		return;

	_dimNum = RegularDim(_dimNum, _dimIn, &dimRegular);

	_outMaze->dimNum_ = _dimNum;
	_outMaze->factorEveryDim_ = (int*)malloc(_dimNum * sizeof(int));//����������ӿռ䣨ʵ�ʼ������ӿ�����һ������Ϊ�������Ӳ����������
	_outMaze->dims_ = (DIMCONTENT)malloc(_dimNum * sizeof(DIMITEM));
	memcpy(_outMaze->dims_, dimRegular, sizeof(int)*_dimNum);
	factorEveryDim = 1;
	_outMaze->factorEveryDim_[_dimNum - 1] = 1;
	for (i = 0; i < _dimNum; i++)
	{
		int currentDimSize = dimRegular[_dimNum - i - 1];//ÿ��ά�ȵĳ߶�
		factorEveryDim *= currentDimSize;

		if (i < _dimNum - 1)
			_outMaze->factorEveryDim_[_dimNum - i - 2] = factorEveryDim;
	}

	totalRoom = factorEveryDim;//���һ�����������������ռ�ķ�������
	_outMaze->rooms_ = (PRoom)malloc(sizeof(Room)*totalRoom);//���䷿��
	memset(_outMaze->rooms_, 0, sizeof(Room)*totalRoom);

	//���㲢��������ǽ��/��
	//ÿ���һ��ά�ȣ�ÿ������������ǽ��/�ţ�ÿ���һ�����䣬��ΪNά���Թ������N��ǽ��/�ţ�������������ǽ��һ�����ȫ����ǽ��/����Ŀ
	//��ǽ������������ά�����ֵ������˵�����
	//��������ǽ��/�ŵ���Ŀ�Ƿ�����������ά����Ŀ+��ǽ����/2
	// // 2018/4/4 ���ϵķ����Ǵ���ģ�������ά�Լ�������Ч����ʱ�����ǽ�ڴ��棬�����϶�������ģ�����ô���
	totalWall = _dimNum * totalRoom;//��ǽ����+��ǽ����/2
	totalOutterWall = 0;//������ǽ����
	for (i = 0; i < _dimNum; i++)
		for (j = 0; j < _dimNum; j++)
			if (i != j)
				totalOutterWall += dimRegular[i] * dimRegular[j];

	//totalWall += totalOutterWall / 2;//��ǽ����/2+��ǽ��

	//totalWall *= 2;
	//totalWall -= totalOutterWall;
	totalWall = _dimNum * 2 * totalRoom;//��ʱ�����ǽ�ڴ��棬�����϶�������ģ������ǹ���ǽ�ڵ����������ô���

	size = sizeof(PDoor)*totalWall;
	_outMaze->doors_ = (PPDoor)malloc(size);
	memset(_outMaze->doors_, 0, size);

	for (i = 0; i < totalWall; i++)
	{
		InitializeDoor(_outMaze->doors_ + i);
	}

	_outMaze->roomNum_ = totalRoom;
	_outMaze->doorNum_ = totalWall;

	currentDim = (DIMCONTENT)malloc(_dimNum*(sizeof(DIMITEM)));
	memset(currentDim, 0, sizeof(DIMITEM)*_dimNum);

	currentRoom = _outMaze->rooms_;
	currentDoor = _outMaze->doors_;
	ThroughAndInitializeAllRoomInMaze(
		_dimNum,
		_dimIn,
		0,
		currentDim,
		_outMaze,
		&currentDoor,
		&currentRoom
	);

	//��ʼ��ÿ�������Լ��������ǵ�ǽ��/��
	//����������з���
	//{
	//	int *currentDim;//��������
	//	unsigned char finished = 0;//����������ʶ
	//	int currentPos = 0;//��ǰ������ά�����
	//	int currentMaxPos = 0;//��ǰ���������ά�����
	//	PDoor currentUsedDoor = NULL;//��ǰʹ�õ����ŵ�λ��

	//	currentDim = (int*)malloc(_dimNum * sizeof(int));
	//	memset(currentDim, 0, sizeof(int)*_dimNum);
	//	do
	//	{
	//		if (currentDim[currentPos] >= dimRegular[currentPos])//�Ѿ���������ǰ����ά����ŵ����ֵ
	//		{
	//			if (currentPos >= _dimNum)//�Ѿ������һ��ά��
	//			{
	//				finished = 1;
	//				break;
	//			}
	//			else
	//			{
	//				currentDim[currentPos + 1]++;
	//				for (j = 0; j < currentPos; j++)//֮ǰ��ά��ȫ������
	//					currentDim[j] = 0;

	//			}
	//		}
	//	} while (finished == 0);
	//}

	//�������ſռ�
	size = (int)(currentDoor - _outMaze->doors_);
	for (i = size; i < _outMaze->doorNum_; i++)
	{
		free(_outMaze->doors_[i]);
	}
	_outMaze->doorNum_ = size;
	currentDoor = _outMaze->doors_;
	size = size * sizeof(PDoor);
	_outMaze->doors_ = (PPDoor)malloc(size);

	memcpy(_outMaze->doors_, currentDoor, size);
	free(currentDoor);
}

//���䲢����һ��ά�����굽�µ�ά������
NONE CopyDim(DIMCONTENT _DimFrom, PDIMCONTENT _DimTo, DIMLENGTH _DimNum)
{
	if (*_DimTo != NULL)
		free(*_DimTo);
	if (_DimFrom != NULL && _DimNum > 0)
	{
		*_DimTo = (DIMCONTENT)malloc(sizeof(DIMCONTENT)*_DimNum);
		memcpy(*_DimTo, _DimFrom, sizeof(DIMITEM)*_DimNum);
	}
	else
		*_DimTo = NULL;
}

DIMCONTENT CopyDimEx(DIMCONTENT _Dim, DIMLENGTH _DimNum)
{
	DIMCONTENT ret = NULL;
	if (_Dim != NULL && _DimNum > 0)
		CopyDim(_Dim, &ret, _DimNum);
	return ret;
}

//�ݹ��������ʼ���Թ������з���
//_DimNum ά�ȵ���Ŀ����_Dim����ĳ���
//_Dim ��ά�ȵĳ߶�
//_currentDimPos ��ǰ������ά�����ڵ�ά��ֵ
//_currentDim ��ǰ����������
//_maze ���淿�䡢�ŵĽṹ
//_currentDoor ��ǰʹ�õ���ָ��
//_currentRoom ��ǰ����ķ���
NONE ThroughAndInitializeAllRoomInMaze(
	DIMLENGTH _DimNum,
	DIMCONTENT _Dim,
	DIMINDEX _currentDimPos,
	DIMCONTENT _CurrentDim,
	PMaze _maze,
	PPDoor* _currentDoor,
	PPRoom _currentRoom)
{
	int i;

	for (i = 0; i < _Dim[_currentDimPos]; i++)
	{
		_CurrentDim[_currentDimPos] = i;
		if (_currentDimPos + 1 >= _DimNum)//ֻ�еݹ鵽���һ��ά�Ȳ��������䷿��
		{
			PRoom room = *_currentRoom;			

			//printf("Now alloc NO. %d room,total room is %d\r\n", (room - _maze->rooms_), _maze->roomNum_);

			room->dim_ = NULL;
			room->Doors_ = NULL;
			room->dimNum_ = _maze->dimNum_;

			room->dim_ = CopyDimEx(_CurrentDim, _DimNum);
			room->flags_ ^= 0x40000000;//��ʶ�÷����Ѿ�����

			//room->dim_[_currentDimPos - 1] = i;
			//printf("%d %d,%d,%d,%d\r\n",
			//	room - _maze->rooms_,
			//	room->dim_[0], room->dim_[1], room->dim_[2], room->dim_[3]
			//);

			(*_currentRoom)++;
			AllocDoorForRoom(_maze, room, _currentDoor);
			//DumpRoom(room);
			//AllocDoorForRoom(room, _maze->doors_, _currentDoor, _DimNum);
		}
		else
			ThroughAndInitializeAllRoomInMaze(
				_DimNum,
				_Dim,
				_currentDimPos + 1,
				_CurrentDim,
				_maze,
				_currentDoor,
				_currentRoom
			);
	}
}

BOOL RoomInitialized(PRoom _room)
{
	return (_room->flags_ & 0x40000000) != 0;
}

NONE MarkDoorUsed(PDoor _door)
{
	_door->flags_ ^= 0x40000000;
}

NONE MarkDoorEdge(PDoor _door)
{
	_door->flags_ ^= 0x00008000;
}

int FindRoomFirstUnUsedDoor(PRoom _room, int _size)
{
	int i;
	PPDoor door_p;
	//printf("FindRoomFirstUnUsedDoor %d\r\n", _size);
	for (i = 0; i < _size; i++)
	{
		door_p = (PPDoor)GetPointerPS(_room->Doors_->doors_, _room->Doors_->count_, i);
		if (door_p != NULL && *door_p == NULL)
		{
			//printf("FindRoomFirstUnUsedDoor return %d\n", i);
			return i;
		}
	}
	return -1;
}

#ifdef _DEBUG
static NONE DumpDim(DIMCONTENT _dim, DIMLENGTH _length)
{
	DIMLENGTH i;
	printf("(");
	for (i = 0; i < _length; i++)
		printf(i == 0 ? "%d" : ",%d", _dim[i]);
	printf(")\r\n");
}
#endif

#ifdef _DEBUG
static NONE DumpDim2(DIMCONTENT _dim, DIMLENGTH _length)
{
	DIMLENGTH i;
	printf("(");
	for (i = 0; i < _length; i++)
		printf(i == 0 ? "%d" : ",%d", _dim[i]);
	printf(")");
}
#endif

#define MsgDim(a,b,c) printf(a);DumpDim(b,c)

//��ָ������ķ���͸����ķ��������������������������õ��ţ���Ҫ�Ļ���_doors�ﴴ��һ������_doorsָ����ǰ�ƶ�һ����
static PDoor LinkRoomByDoor(DIMLENGTH _dimLength, DIMCONTENT _dimFrom, PRoom _roomFrom, DIMCONTENT _dimTo, PPDoor* _currentDoor, PMaze _maze)
{
	int size, i;//���ֳ߶�������ʱ����
	int flagNewDoors = 0;//���ڵķ����Ƿ�������´��������ӵ��ţ������ڴ˷�����ң�
	///ToDo
	//���ȶ�λ���ӵ��ķ���
	PRoom roomTo = FindRoom(_dimLength, _dimTo, _maze);
	if (roomTo == NULL)
		return NULL;//�޷���λ����

#ifdef _DEBUG2
	printf("link from ");
	OutDim(_dimFrom, _dimLength);
	printf("to");
	OutDim(_dimTo, _dimLength);
#endif

	size = _maze->dimNum_ * 2;//�����������Ŀ
	//

	//������ڵķ������Ϊ�գ�˵���������δ����
	if (roomTo->Doors_ == NULL)//��������ڵķ�������ſռ䣬��ʹ���������δ��ʼ��
		flagNewDoors = 1;

	int unusedDoorStartPositionForNeighbor = -1, foundLinkDoor = 0;//��һ��δʹ�õ��ŵĿ�ʼλ��(�ھ�)���Ƿ��ҵ������ӵ��ű�ʶ
	int unusedDoorStartPosition = -1;//��һ��δʹ�õ��ŵĿ�ʼλ�ã�����
	PDoor existLinkDoor = NULL;
	//������ڵķ����Ѿ���ʼ����������������Ҳ������Ӵ˷�����ţ�Ҳ˵���������δ����

	if (!flagNewDoors)
		for (i = 0; i < size; i++)//���Ҹ÷������е������Ƿ�������ԭ�������
		{
			PPDoor door_p = (PPDoor)GetPointerPS(roomTo->Doors_->doors_, roomTo->Doors_->count_, i);
			if (door_p != NULL && *door_p != NULL)
			{
				if (foundLinkDoor == 0)//��û���ҵ������ӵ���
				{
					PDoor door = *door_p;
					if (door->linkedRooms_[0] == _roomFrom
						||
						door->linkedRooms_[1] == _roomFrom)
					{
						foundLinkDoor = 1;//��ʶ���ҵ����ӵ���
						existLinkDoor = door;
					}
				}
			}
			else
			{
				unusedDoorStartPositionForNeighbor = i;
				break;//�ҵ��˿ɷ���ķ���
			}
		}
	else
	{
		roomTo->Doors_ = (PDoorSet)malloc(sizeof(DoorSet));

		InitializePS(&roomTo->Doors_->doors_, &roomTo->Doors_->count_);
		AllocPS(&roomTo->Doors_->doors_, &roomTo->Doors_->count_, size);

		unusedDoorStartPositionForNeighbor = 0;
	}

	if (!foundLinkDoor)//û������ԭ�������
	{
		//����������δ�����������Թ���������doors_��������һ����Ϊ��ǰ��
		unusedDoorStartPosition = FindRoomFirstUnUsedDoor(_roomFrom, size);
		assert(unusedDoorStartPositionForNeighbor >= 0);//��������У���������Assert
		assert(unusedDoorStartPosition >= 0);
		{
			//���Թ����������һ���Ÿ���ԭ�����������ڵķ���
			PDoor newDoor = **_currentDoor;
			PPDoor door_p = NULL;
			(*_currentDoor)++;

#ifdef _DEBUG2
			printf("Link Door: From");
			DumpDim2(_roomFrom->dim_, _dimLength);
			printf("To ");
			DumpDim2(_dimTo, _dimLength);
			printf("\r\n");
#endif

			MarkDoorUsed(newDoor);//��ʶ�÷����Ѿ�������
			newDoor->isClosed_ = 1;//�ȹ�����
			newDoor->linkedRooms_[0] = roomTo;
			newDoor->linkedRooms_[1] = _roomFrom;

			door_p = (PPDoor)GetPointerPS(roomTo->Doors_->doors_, roomTo->Doors_->count_, unusedDoorStartPositionForNeighbor);
			assert(door_p != NULL);
			*door_p = newDoor;//�����ڷ���ĵ�һ���ɷ����Ÿ��µ���0

			door_p = (PPDoor)GetPointerPS(_roomFrom->Doors_->doors_, _roomFrom->Doors_->count_, unusedDoorStartPosition);
			assert(door_p != NULL);
			*door_p = newDoor;
			return newDoor;
		}
	}
	else//������ԭ�������
	{
		assert(foundLinkDoor != 0);
		assert(existLinkDoor != NULL);
		assert(existLinkDoor->linkedRooms_[0] == NULL || existLinkDoor->linkedRooms_[1] == NULL
			|| existLinkDoor->linkedRooms_[0] == _roomFrom || existLinkDoor->linkedRooms_[1] == _roomFrom);

		PPDoor door_p = NULL;

		if (existLinkDoor->linkedRooms_[0] == NULL && existLinkDoor->linkedRooms_[1] != _roomFrom)
			existLinkDoor->linkedRooms_[0] = _roomFrom;
		else if (existLinkDoor->linkedRooms_[1] == NULL && existLinkDoor->linkedRooms_[0] != _roomFrom)
			existLinkDoor->linkedRooms_[1] = _roomFrom;


		//Ҳ����Ҫ�ҵ���һ���ɷ�����ŵ�λ��
		unusedDoorStartPosition = FindRoomFirstUnUsedDoor(_roomFrom, size);
		door_p = (PPDoor)GetPointerPS(_roomFrom->Doors_->doors_, _roomFrom->Doors_->count_, unusedDoorStartPosition);
		assert(door_p != NULL);
		*door_p = existLinkDoor;
		return existLinkDoor;
	}
	//���ڵķ����Ѿ���ʼ��������������ҵ������Ӵ˷�����ţ��򽫴��ż��뱾�����������

}

// ������ά�ռ��Լ����ϵĸ����Դ�����
// �����ƶ�ÿ����һ��ά�ȣ�һ������ֻ�࿪������
// ���ط����������
static NONE AllocDoorForRoom(PMaze _maze, PRoom _room, PPDoor* _currentDoor)
{
	//ÿ��������ŵ������ĿΪά��*2
	DIMLENGTH dimNum = _maze->dimNum_;
	DIMCONTENT dims = NULL;
	DIMINDEX i;	
	int size;

	dims = CopyDimEx(_maze->dims_, dimNum);

	//���û�з�����ţ�������Ԥ�����ţ�������������ΪNULL
	if (_room->Doors_ == NULL) {
		_room->Doors_ = (PDoorSet)malloc(sizeof(DoorSet));
		size = dimNum * 2 * sizeof(PDoor);
		InitializePS(&_room->Doors_->doors_, &_room->Doors_->count_);
		AllocPS(&_room->Doors_->doors_, &_room->Doors_->count_, dimNum * 2);
	}

	//MsgDim("Alloc Door For", _room->dim_, dimNum);

	///Ԥ�������ڷ�������ռ�
	//size = dimNum * 2 * sizeof(int*); //temp
	//neighborRoomsDim = malloc(size);
	//memset(neighborRoomsDim, 0, size);

	//���ݵ�ǰ�����ڸ���ά�ȷ����ϼӼ�һ��ȡ���������ڷ���,��ȡ��������//�󣬴���neighborRoomsDim
	for (i = 0; i < dimNum; i++)
	{
		DIMCONTENT dimLinkRoom = NULL;
		dimLinkRoom = CopyDimEx(_room->dim_, dimNum);
		//MsgDim("Build Link For", dimLinkRoom, dimNum);

		if (dimLinkRoom[i] > 0)//���iά���ϸ÷��䲻����С�߽���
		{
			DIMCONTENT dimLinkRoomReduce = NULL;
			dimLinkRoomReduce = CopyDimEx(dimLinkRoom, dimNum);
			dimLinkRoomReduce[i]--;
			//neighborRoomsDim[i * 2] = dimLinkRoomReduce;
			LinkRoomByDoor(dimNum, dimLinkRoom, _room, dimLinkRoomReduce, _currentDoor, _maze);

			free(dimLinkRoomReduce);
		}
		else
		{
			//��������ڿռ�߽������ű��Ϊ�߽�Edge�����ٽ�������
			int firstUnusedDoorPosition = FindRoomFirstUnUsedDoor(_room, dimNum * 2);
			PDoor door = **_currentDoor;
			PPDoor door_p = NULL;
			*_currentDoor = *_currentDoor + 1;
			MarkDoorEdge(door);
			MarkDoorUsed(door);

#ifdef _DEBUG2
			printf("Edge Wall:");
			DumpDim(_room->dim_, dimNum);
#endif
			door->isClosed_ = 1;
			door->linkedRooms_[0] = _room;
			door_p = (PPDoor)GetPointerPS(_room->Doors_->doors_, _room->Doors_->count_, firstUnusedDoorPosition);

			assert(door_p != NULL);
			*door_p = door;
		}

		if (dimLinkRoom[i] < dims[i] - 1)//���iά���ϸ÷��䲻�����߽���
		{
			DIMCONTENT dimLinkRoomIncrease = NULL;
			dimLinkRoomIncrease = CopyDimEx(dimLinkRoom, dimNum);
			dimLinkRoomIncrease[i]++;
			//neighborRoomsDim[i * 2 + 1] = dimLinkRoomIncrease;
			LinkRoomByDoor(dimNum, dimLinkRoom, _room, dimLinkRoomIncrease, _currentDoor, _maze);

			free(dimLinkRoomIncrease);
		}
		else
		{
			//�߽��ϵ���
			//��������ڿռ�߽������ű��Ϊ�߽�Edge�����ٽ�������
			int firstUnusedDoorPosition = FindRoomFirstUnUsedDoor(_room, dimNum * 2);
			PDoor door = **_currentDoor;
			PPDoor door_p = NULL;
			*_currentDoor = *_currentDoor + 1;
			MarkDoorEdge(door);
			MarkDoorUsed(door);
#ifdef _DEBUG2
			printf("Edge Wall:");
			DumpDim(_room->dim_, dimNum);
#endif
			door->isClosed_ = 1;
			door->linkedRooms_[0] = _room;
			door_p = (PPDoor)GetPointerPS(_room->Doors_->doors_, _room->Doors_->count_, firstUnusedDoorPosition);

			assert(door_p != NULL);
			*door_p = door;
		}

		free(dimLinkRoom);
	}

	//free(neighborRoomsDim);

	free(dims);
}

//��_maze��Ѱ������Ϊ_dim�ķ���
//<image url="..\��ά���������Դ洢�еĶ�λ3.png"/>
PRoom FindRoom(DIMLENGTH _dimNum, DIMCONTENT _dim, PMaze _maze)
{
	int i, pos = 0;
	int* dim = NULL;
	dim = CopyDimEx(_dim, _dimNum);

	RegularDimForMaze(_maze, &dim, _dimNum);
	for (i = 0; i < _maze->dimNum_ - 1; i++)
	{
		pos += dim[i] * _maze->factorEveryDim_[i];
	}

	PRoom ret = _maze->rooms_ + pos + dim[i];
	free(dim);
	return ret;
}

//�����Թ�����߶ȹ��������γ������
//_maze ��Ϊ��׼���Թ���������ά�ȵ���Ŀ�͸�ά�ȵĳ߶�
// _dim �����ά������
// _inputDimNum �����ά�����곤�ȣ�������������Ŀ
NONE RegularDimForMaze(PMaze _maze, PDIMCONTENT _dim, DIMLENGTH _inputDimNum)
{
	DIMINDEX i;
	int* outputDim = (int*)malloc(sizeof(int)*(_maze->dimNum_));
	for (i = 0; i < _maze->dimNum_; i++)
	{
		if (i + 1 > _inputDimNum)//�����ά������ά����Ŀ���㣬��ÿ��ά�ȵĵ�һ������(0)���
			outputDim[i] = 0;
		else if (*_dim == NULL)
			outputDim[i] = 0;
		else if ((*_dim)[i] > _maze->dims_[i])//�����γ��������ĳһά��Խ�磬ʹ�ø�ά�ȵ���������滻
			outputDim[i] = _maze->dims_[i];
		else if ((*_dim)[i] < 0)//��ά�ȵ�����Ϊ������0���
			outputDim[i] = 0;
		else
			outputDim[i] = (*_dim)[i];
	}
	if (*_dim != NULL)
		free(*_dim);
	*_dim = outputDim;
}

//����ά������ʹ��ά�������壬����ʵ���������ά����Ŀ
DIMLENGTH RegularDim(DIMLENGTH _dimNum, DIMCONTENT _dimIn, PDIMCONTENT _dimOut)
{
	DIMINDEX i;
	int* buffer;
	int unusedDimCount = 0;
	buffer = (DIMCONTENT)malloc(sizeof(DIMITEM)*_dimNum);
	for (i = 0; i < _dimNum; i++)
	{
		if (_dimIn[i] <= 0)
			unusedDimCount++;
		else
			buffer[i - unusedDimCount] = _dimIn[i];
	}

	*_dimOut = buffer;
	return _dimNum - unusedDimCount;
}

NONE MarkRoomDestroyed(PRoom _room)
{
	_room->flags_ &= 0x08000000;
}

NONE MarkDoorDestroyed(PDoor _door)
{
	_door->flags_ &= 0x08000000;
}

NONE FreeRoom(PRoom _room)
{
	if (_room->dim_ != NULL)
		free(_room->dim_);
	_room->dim_ = NULL;

	FreeDoorSet(_room->Doors_);

	if (_room->Doors_ != NULL)
		free(_room->Doors_);
	_room->Doors_ = NULL;

	MarkRoomDestroyed(_room);
}

NONE FreeMaze(PMaze _maze)
{
	if (_maze->dims_ != NULL)
		free(_maze->dims_);

	_maze->dims_ = NULL;

	if (_maze->rooms_ != NULL)
	{
		int i;

		for (i = 0; i < _maze->roomNum_; i++)
			FreeRoom(&(_maze->rooms_[i]));

		free(_maze->rooms_);
		_maze->rooms_ = NULL;
		_maze->roomNum_ = 0;
	}

	if (_maze->doors_ != NULL)
	{
		int i;
		for (i = 0; i < _maze->doorNum_; i++)
		{
			MarkDoorDestroyed(_maze->doors_[i]);
			free(_maze->doors_[i]);
		}

		free(_maze->doors_);
		_maze->doors_ = NULL;
		_maze->doorNum_ = 0;
	}

	if (_maze->factorEveryDim_ != NULL)
	{
		free(_maze->factorEveryDim_);
		_maze->factorEveryDim_ = NULL;
	}
}

DIMCONTENT GenerateDefaultDim(PMaze _maze)
{
	int* ret;
	DIMINDEX size, i;

	size = _maze->dimNum_;
	ret = (DIMCONTENT)malloc(sizeof(DIMITEM)*size);
	for (i = 0; i < size; i++)
	{
		ret[i] = 0;
	}
	return ret;
}
//������·����������

//static PRoom RemoveRoomFromPath(PPRoom _path, int _pos, int* _size)
//{
//	PRoom ret;
//	PPRoom buffer;
//	int mvSize = *_size - _pos - 1;
//	assert(_pos < *_size && _pos >= 0);
//	ret = _path[_pos];
//	if (mvSize > 0)
//	{
//		mvSize *= sizeof(PRoom);
//		buffer = (PPRoom)malloc(mvSize);
//		memcpy(buffer, _path + _pos + 1, mvSize);
//		memcpy(_path + _pos, buffer, mvSize);
//		free(buffer);
//	}
//
//	*_size--;
//	return ret;
//}
//
//static void InsertRoomToPath(PPRoom _path, int _pos, PRoom _insert, int* _size)
//{
//	int i, size;
//	PPRoom posInsert = NULL;
//	for (i = 0; i < *_size; i++)
//	{
//
//	}
//}

NONE DumpRoom(PRoom _room)
{
	printf("Room@");
	DumpDim(_room->dim_, _room->dimNum_);
	for (int i = 0; i < _room->Doors_->count_; i++)
	{
		DumpDoor(_room->Doors_->doors_[i], _room);
	}

	printf("\r\nEnd\r\n");
}

NONE DumpDoor(PDoor _door, PRoom _roomIgnore)
{
	printf("==>Door:");
	if (_door == NULL)
		printf("NULL");
	else
	{
		//if (_door->linkedRooms_[0] != NULL && _door->linkedRooms_[0] != _roomIgnore)
		if (_door->linkedRooms_[0] == _roomIgnore)
			printf("SELF");
		else if (_door->linkedRooms_[0] != NULL)
			DumpDim2(_door->linkedRooms_[0]->dim_, _door->linkedRooms_[0]->dimNum_);
		else
			printf("NULL");
		if (_door->isClosed_ == 0)
			printf("<->");
		else
			printf("<X>");
		//if (_door->linkedRooms_[1] != NULL && _door->linkedRooms_[1] != _roomIgnore)
		if (_door->linkedRooms_[1] == _roomIgnore)
			printf("SELF");
		else if (_door->linkedRooms_[1] != NULL)
			DumpDim2(_door->linkedRooms_[1]->dim_, _door->linkedRooms_[1]->dimNum_);
		else
			printf("NULL");
	}
	printf("<==\r\n");
}

RoomSet* AllocRoomSet()
{
	RoomSet* ret;
	ret = (RoomSet*)malloc(sizeof(RoomSet));
	ret->count_ = 0;
	ret->rooms_ = NULL;
	return ret;
}

NONE FreeRoomSet(RoomSet** _roomSet)
{
	if (_roomSet == NULL)
		return;
	if ((*_roomSet)->rooms_ != NULL)
		free((*_roomSet)->rooms_);

	(*_roomSet)->rooms_ = NULL;
	free(*_roomSet);
	*_roomSet = NULL;
}

PDoorSet CreateDoorSet()
{
	PDoorSet ret;
	ret = (PDoorSet)malloc(sizeof(DoorSet));
	ret->count_ = 0;
	ret->doors_ = NULL;
	return ret;
}

NONE InitializeDoorSet(PDoorSet _ret)
{
	_ret->count_ = 0;
	_ret->doors_ = NULL;
}

NONE AllocDoorSet(PDoorSet _doors, int _doorNum)
{
	//FreeDoorSet(_doors);//��������ͷŰ�������

	int size = sizeof(PDoor)*_doorNum;
	_doors->count_ = _doorNum;
	_doors->doors_ = (PPDoor)malloc(size);
	memset(_doors->doors_, 0, size);
}


NONE FreeDoorSet(PDoorSet _doorSet)
{
	if (_doorSet == NULL)
		return;
	PDoorSet doorset = _doorSet;
	if (doorset->doors_ != NULL)
		free(doorset->doors_);
	doorset->doors_ = NULL;

	free(_doorSet);
	_doorSet = NULL;
}


RoomSet* GetNeighborRooms(PRoom _room, int _dim)
{
	RoomSet* ret = AllocRoomSet();
	PDoorSet doorset = GetRoomDoorsEx(_room, _dim);
	PPRoom buffer;
	int size, i, count = 0;

	size = doorset->count_;
	buffer = (PPRoom)malloc(size * sizeof(PRoom));
	for (i = 0; i < size; i++)
	{
		PDoor door = doorset->doors_[i];
		if (door->linkedRooms_[0] == _room && door->linkedRooms_[1] != NULL)
		{
			buffer[count++] = door->linkedRooms_[1];
		}
		else if (door->linkedRooms_[1] == _room && door->linkedRooms_[0] != NULL)
		{
			buffer[count++] = door->linkedRooms_[0];
		}
	}

	size = count * sizeof(PRoom);
	ret->count_ = count;
	ret->rooms_ = (PPRoom)malloc(size);
	memcpy(ret->rooms_, buffer, size);
	FreeDoorSet(doorset);
	return ret;
}

//��÷������е��� �����Թ�����
PDoorSet GetRoomDoors(PRoom _room, PMaze _maze)
{
	return GetRoomDoorsEx(_room, _maze->dimNum_);
}

//��ȡ�������е��ţ�����ռ�ά����Ŀ
PDoorSet GetRoomDoorsEx(PRoom _room, DIMLENGTH _dim)
{
	PDoorSet ret = CreateDoorSet();
	PPDoor doors = _room->Doors_->doors_, buffer;
	int count = 0, i, size = _room->Doors_->count_;

	buffer = (PPDoor)malloc(size * sizeof(PDoor));

	for (i = 0; i < size; i++)
	{
		if (doors[i] != NULL)
		{
			buffer[i] = doors[i];
			count++;
		}
	}

	if (count > 0)
	{
		size = count * sizeof(PDoor);
		ret->count_ = count;
		ret->doors_ = (PPDoor)malloc(size);
		memcpy(ret->doors_, buffer, size);
	}
	else
	{
		ret->count_ = 0;
		ret->doors_ = NULL;
	}
	free(buffer);

	return ret;
}


STRING DimValue(PDim _dim)
{
#define BUFFER_LENGTH 40
	static char buffer[BUFFER_LENGTH] = "\0";
	for (DIMINDEX i = 0, p = 0; i < _dim->length; i++)
	{
		if (p < BUFFER_LENGTH - 1)
		{
			p += sprintf_s(buffer + p, BUFFER_LENGTH - p, "%d", _dim->values_[i]);
		}
		if (p < BUFFER_LENGTH - 1 && i != _dim->length - 1)
			buffer[p++] = '.';
	}
	return buffer;
#undef BUFFER_LENGTH
}

STRING DimValue2(DIMCONTENT _dim, DIMLENGTH _length)
{
#define BUFFER_LENGTH 40
	static char buffer[BUFFER_LENGTH] = "\0";
	for (int i = 0, p = 0; i < _length; i++)
	{
		if (p < BUFFER_LENGTH - 1)
		{
			p += sprintf_s(buffer + p, BUFFER_LENGTH - p, "%d", _dim[i]);
		}
		if (p < BUFFER_LENGTH - 1 && i != _length - 1)
			buffer[p++] = '.';
	}
	return buffer;
#undef BUFFER_LENGTH
}

NONE DumpRoomSet(PPRoom _rooms, int _size)
{
	if (_rooms != NULL && _size > 0)
	{
		for (int i = 0; i < _size; i++)
		{
			if (_rooms[i] != NULL)
			{
				PRoom room = _rooms[i];
				DumpDim2(room->dim_, room->dimNum_);
			}
			else
			{
				printf("(nil)");
			}
		}
	}
	printf("\r\n");
}

NONE DumpDoorSet(PPDoor _doors, int _size)
{
	if (_doors != NULL)
	{
		for (int i = 0; i < _size; i++)
		{
			DumpDoor(_doors[i], NULL);
		}
	}
}