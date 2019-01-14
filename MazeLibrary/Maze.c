#include <stdlib.h>
#include <memory.h>
#include <malloc.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "Maze.h"
#include "PointerSet.h"

//递归遍历并初始化迷宫中所有房间
//_DimNum 维度的数目，即_Dim数组的长度
//_Dim 各维度的尺度
//_currentDimPos 当前遍历的维度所在的维度值
//_currentDim 当前遍历的坐标
//_maze 保存房间、门的结构
//_currentDoor 当前使用的门指针
//_currentRoom 当前分配的房间
static void ThroughAndInitializeAllRoomInMaze(
	DIMLENGTH _DimNum,
	DIMCONTENT _Dim,
	DIMINDEX _currentDimPos,
	DIMCONTENT _CurrentDim,
	PMaze _maze,
	PPDoor* _currentDoor,
	PPRoom _currentRoom);
//为房间分配所有的门
// _maze 迷宫
// _room 当前分配门所在的房间
// _currentDoor 当前已经分配使用的门的位置
// 由于四维空间以及以上的复杂性大大提高
// 所以制定每增加一个维度，一个立方体的房间只多开两个门
// 增加门的次序为先上下、再左右、前后
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

//检验门是否启用
BOOL isDoorCreated(PDoor _door)
{
	return (_door->flags_ & 0x40000000) != 0;
}

NONE AllocMaze(DIMLENGTH _dimNum, DIMCONTENT _dimIn, PMaze _outMaze)
{
	//assert(_dimIn != NULL && _dimNum > 0);
	//首先要整理维度
	DIMCONTENT dimRegular;
	int i, j, factorEveryDim;
	int totalRoom;//房间总数
	int totalWall;//墙壁/门总数
	int totalOutterWall;//外墙总数
	DIMCONTENT currentDim;//用以初始化房间的维度坐标
	PRoom currentRoom;
	Door **currentDoor;
	int size;

	if (_outMaze == NULL)
		return;

	_dimNum = RegularDim(_dimNum, _dimIn, &dimRegular);

	_outMaze->dimNum_ = _dimNum;
	_outMaze->factorEveryDim_ = (int*)malloc(_dimNum * sizeof(int));//分配计算因子空间（实际计算因子可以少一个，因为最后的因子不会参与运算
	_outMaze->dims_ = (DIMCONTENT)malloc(_dimNum * sizeof(DIMITEM));
	memcpy(_outMaze->dims_, dimRegular, sizeof(int)*_dimNum);
	factorEveryDim = 1;
	_outMaze->factorEveryDim_[_dimNum - 1] = 1;
	for (i = 0; i < _dimNum; i++)
	{
		int currentDimSize = dimRegular[_dimNum - i - 1];//每个维度的尺度
		factorEveryDim *= currentDimSize;

		if (i < _dimNum - 1)
			_outMaze->factorEveryDim_[_dimNum - i - 2] = factorEveryDim;
	}

	totalRoom = factorEveryDim;//最后一个因子正好是整个空间的房间总数
	_outMaze->rooms_ = (PRoom)malloc(sizeof(Room)*totalRoom);//分配房间
	memset(_outMaze->rooms_, 0, sizeof(Room)*totalRoom);

	//计算并分配所有墙壁/门
	//每多出一个维度，每个房间多出两堵墙壁/门，每多出一个房间，作为N维的迷宫将多出N个墙壁/门，这样，加上外墙的一半就是全部的墙壁/门数目
	//外墙的总数是所有维度最大值两两相乘的两倍
	//所以所有墙壁/门的数目是房间总数乘以维度数目+外墙总数/2
	// // 2018/4/4 以上的分析是错误的，仅在三维以及以下有效，暂时以最大墙壁代替，这样肯定是冗余的，先这么办吧
	totalWall = _dimNum * totalRoom;//内墙总数+外墙总数/2
	totalOutterWall = 0;//计算外墙总数
	for (i = 0; i < _dimNum; i++)
		for (j = 0; j < _dimNum; j++)
			if (i != j)
				totalOutterWall += dimRegular[i] * dimRegular[j];

	//totalWall += totalOutterWall / 2;//外墙总数/2+内墙数

	//totalWall *= 2;
	//totalWall -= totalOutterWall;
	totalWall = _dimNum * 2 * totalRoom;//暂时以最大墙壁代替，这样肯定是冗余的，不考虑共用墙壁的情况，先这么办吧

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

	//初始化每个房间以及分配它们的墙壁/门
	//逐个遍历所有房间
	//{
	//	int *currentDim;//遍历坐标
	//	unsigned char finished = 0;//遍历结束标识
	//	int currentPos = 0;//当前递增的维度序号
	//	int currentMaxPos = 0;//当前递增的最大维度序号
	//	PDoor currentUsedDoor = NULL;//当前使用到的门的位置

	//	currentDim = (int*)malloc(_dimNum * sizeof(int));
	//	memset(currentDim, 0, sizeof(int)*_dimNum);
	//	do
	//	{
	//		if (currentDim[currentPos] >= dimRegular[currentPos])//已经遍历到当前递增维度序号的最大值
	//		{
	//			if (currentPos >= _dimNum)//已经是最后一个维度
	//			{
	//				finished = 1;
	//				break;
	//			}
	//			else
	//			{
	//				currentDim[currentPos + 1]++;
	//				for (j = 0; j < currentPos; j++)//之前的维度全部置零
	//					currentDim[j] = 0;

	//			}
	//		}
	//	} while (finished == 0);
	//}

	//调整房门空间
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

//分配并拷贝一个维度坐标到新的维度坐标
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

//递归遍历并初始化迷宫中所有房间
//_DimNum 维度的数目，即_Dim数组的长度
//_Dim 各维度的尺度
//_currentDimPos 当前遍历的维度所在的维度值
//_currentDim 当前遍历的坐标
//_maze 保存房间、门的结构
//_currentDoor 当前使用的门指针
//_currentRoom 当前分配的房间
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
		if (_currentDimPos + 1 >= _DimNum)//只有递归到最后一个维度才真正分配房间
		{
			PRoom room = *_currentRoom;			

			//printf("Now alloc NO. %d room,total room is %d\r\n", (room - _maze->rooms_), _maze->roomNum_);

			room->dim_ = NULL;
			room->Doors_ = NULL;
			room->dimNum_ = _maze->dimNum_;

			room->dim_ = CopyDimEx(_CurrentDim, _DimNum);
			room->flags_ ^= 0x40000000;//标识该房间已经构建

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

//将指定坐标的房间和给定的房间用门连接起来，返回连接用的门，需要的话从_doors里创建一个并将_doors指针向前移动一个门
static PDoor LinkRoomByDoor(DIMLENGTH _dimLength, DIMCONTENT _dimFrom, PRoom _roomFrom, DIMCONTENT _dimTo, PPDoor* _currentDoor, PMaze _maze)
{
	int size, i;//各种尺度所用临时变量
	int flagNewDoors = 0;//相邻的房间是否在这次新创建了连接的门（不用在此房间查找）
	///ToDo
	//首先定位连接到的房间
	PRoom roomTo = FindRoom(_dimLength, _dimTo, _maze);
	if (roomTo == NULL)
		return NULL;//无法定位房间

#ifdef _DEBUG2
	printf("link from ");
	OutDim(_dimFrom, _dimLength);
	printf("to");
	OutDim(_dimTo, _dimLength);
#endif

	size = _maze->dimNum_ * 2;//房间最多门数目
	//

	//如果相邻的房间的门为空，说明这个门尚未建立
	if (roomTo->Doors_ == NULL)//给这个相邻的房间分配门空间，即使这个房间尚未初始化
		flagNewDoors = 1;

	int unusedDoorStartPositionForNeighbor = -1, foundLinkDoor = 0;//第一个未使用的门的开始位置(邻居)，是否找到已连接的门标识
	int unusedDoorStartPosition = -1;//第一个未使用的门的开始位置（自身）
	PDoor existLinkDoor = NULL;
	//如果相邻的房间已经初始化，但房间的门里找不到连接此房间的门，也说明这个门尚未建立

	if (!flagNewDoors)
		for (i = 0; i < size; i++)//查找该房间所有的门里是否有连接原房间的门
		{
			PPDoor door_p = (PPDoor)GetPointerPS(roomTo->Doors_->doors_, roomTo->Doors_->count_, i);
			if (door_p != NULL && *door_p != NULL)
			{
				if (foundLinkDoor == 0)//还没有找到已连接的门
				{
					PDoor door = *door_p;
					if (door->linkedRooms_[0] == _roomFrom
						||
						door->linkedRooms_[1] == _roomFrom)
					{
						foundLinkDoor = 1;//标识已找到连接的门
						existLinkDoor = door;
					}
				}
			}
			else
			{
				unusedDoorStartPositionForNeighbor = i;
				break;//找到了可分配的房间
			}
		}
	else
	{
		roomTo->Doors_ = (PDoorSet)malloc(sizeof(DoorSet));

		InitializePS(&roomTo->Doors_->doors_, &roomTo->Doors_->count_);
		AllocPS(&roomTo->Doors_->doors_, &roomTo->Doors_->count_, size);

		unusedDoorStartPositionForNeighbor = 0;
	}

	if (!foundLinkDoor)//没有连接原房间的门
	{
		//如果这个门尚未建立，则在迷宫的所有门doors_里分配最后一个作为当前门
		unusedDoorStartPosition = FindRoomFirstUnUsedDoor(_roomFrom, size);
		assert(unusedDoorStartPositionForNeighbor >= 0);//这个必须有，考虑做个Assert
		assert(unusedDoorStartPosition >= 0);
		{
			//从迷宫的门里分配一个门赋予原房间和这个相邻的房间
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

			MarkDoorUsed(newDoor);//标识该房间已经被分配
			newDoor->isClosed_ = 1;//先关上门
			newDoor->linkedRooms_[0] = roomTo;
			newDoor->linkedRooms_[1] = _roomFrom;

			door_p = (PPDoor)GetPointerPS(roomTo->Doors_->doors_, roomTo->Doors_->count_, unusedDoorStartPositionForNeighbor);
			assert(door_p != NULL);
			*door_p = newDoor;//赋相邻房间的第一个可分配门给新的门0

			door_p = (PPDoor)GetPointerPS(_roomFrom->Doors_->doors_, _roomFrom->Doors_->count_, unusedDoorStartPosition);
			assert(door_p != NULL);
			*door_p = newDoor;
			return newDoor;
		}
	}
	else//有链接原房间的门
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


		//也还是要找到第一个可分配的门的位置
		unusedDoorStartPosition = FindRoomFirstUnUsedDoor(_roomFrom, size);
		door_p = (PPDoor)GetPointerPS(_roomFrom->Doors_->doors_, _roomFrom->Doors_->count_, unusedDoorStartPosition);
		assert(door_p != NULL);
		*door_p = existLinkDoor;
		return existLinkDoor;
	}
	//相邻的房间已经初始化，房间的门里找到了连接此房间的门，则将此门加入本房间的门数组

}

// 由于四维空间以及以上的复杂性大大提高
// 所以制定每增加一个维度，一个房间只多开两个门
// 返回房间的所有门
static NONE AllocDoorForRoom(PMaze _maze, PRoom _room, PPDoor* _currentDoor)
{
	//每个房间的门的最大数目为维度*2
	DIMLENGTH dimNum = _maze->dimNum_;
	DIMCONTENT dims = NULL;
	DIMINDEX i;	
	int size;

	dims = CopyDimEx(_maze->dims_, dimNum);

	//如果没有分配过门，给房间预分配门，并将所有门置为NULL
	if (_room->Doors_ == NULL) {
		_room->Doors_ = (PDoorSet)malloc(sizeof(DoorSet));
		size = dimNum * 2 * sizeof(PDoor);
		InitializePS(&_room->Doors_->doors_, &_room->Doors_->count_);
		AllocPS(&_room->Doors_->doors_, &_room->Doors_->count_, dimNum * 2);
	}

	//MsgDim("Alloc Door For", _room->dim_, dimNum);

	///预分配相邻房间坐标空间
	//size = dimNum * 2 * sizeof(int*); //temp
	//neighborRoomsDim = malloc(size);
	//memset(neighborRoomsDim, 0, size);

	//根据当前房间在各个维度方向上加减一获取门所在相邻房间,获取所有坐标//后，存入neighborRoomsDim
	for (i = 0; i < dimNum; i++)
	{
		DIMCONTENT dimLinkRoom = NULL;
		dimLinkRoom = CopyDimEx(_room->dim_, dimNum);
		//MsgDim("Build Link For", dimLinkRoom, dimNum);

		if (dimLinkRoom[i] > 0)//如果i维度上该房间不在最小边界上
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
			//如果房间在空间边界上则将门标记为边界Edge，不再进行连接
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

		if (dimLinkRoom[i] < dims[i] - 1)//如果i维度上该房间不在最大边界上
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
			//边界上的门
			//如果房间在空间边界上则将门标记为边界Edge，不再进行连接
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

//在_maze里寻找坐标为_dim的房间
//<image url="..\多维数组在线性存储中的定位3.png"/>
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

//根据迷宫自身尺度规整输入的纬度坐标
//_maze 作为标准的迷宫，包含了维度的数目和各维度的尺度
// _dim 输入的维度坐标
// _inputDimNum 输入的维度坐标长度，即输入坐标数目
NONE RegularDimForMaze(PMaze _maze, PDIMCONTENT _dim, DIMLENGTH _inputDimNum)
{
	DIMINDEX i;
	int* outputDim = (int*)malloc(sizeof(int)*(_maze->dimNum_));
	for (i = 0; i < _maze->dimNum_; i++)
	{
		if (i + 1 > _inputDimNum)//输入的维度坐标维度数目不足，用每个维度的第一个坐标(0)填充
			outputDim[i] = 0;
		else if (*_dim == NULL)
			outputDim[i] = 0;
		else if ((*_dim)[i] > _maze->dims_[i])//输入的纬度坐标在某一维上越界，使用该维度的最大坐标替换
			outputDim[i] = _maze->dims_[i];
		else if ((*_dim)[i] < 0)//该维度的坐标为负，用0替代
			outputDim[i] = 0;
		else
			outputDim[i] = (*_dim)[i];
	}
	if (*_dim != NULL)
		free(*_dim);
	*_dim = outputDim;
}

//规整维度坐标使各维均有意义，返回实际有意义的维度数目
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
//以下是路径建立部分

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
	//FreeDoorSet(_doors);//不能随便释放啊！！！

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

//获得房间所有的门 传入迷宫参数
PDoorSet GetRoomDoors(PRoom _room, PMaze _maze)
{
	return GetRoomDoorsEx(_room, _maze->dimNum_);
}

//获取房间所有的门，传入空间维度数目
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