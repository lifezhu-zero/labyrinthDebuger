#pragma once

#include "typedef.h"

#ifndef _MAZE_DEFINE_H_
#define _MAZE_DEFINE_H_

typedef struct tagDim
{
	DIMCONTENT values_;
	DIMLENGTH length;
}Dim,*PDim,**PPDim;

typedef struct tagRoom
{
	unsigned long flags_;
	struct tagDoorSet* Doors_;
	DIMLENGTH dimNum_;//维度的数目
	DIMCONTENT dim_;//房间自身的维度，用于调试，为节省空间可以去除
}Room, *PRoom, **PPRoom;

typedef struct tagDoor
{
	unsigned long flags_;
	PRoom linkedRooms_[2];
	unsigned char isClosed_;
}Door, *PDoor, **PPDoor;

typedef struct tagMaze
{
	PRoom rooms_;
	PPDoor doors_;
	int* factorEveryDim_;//每个维度的计算因子，为方便查找定位
	DIMCONTENT dims_;
	DIMLENGTH dimNum_;//维度的数目
	int roomNum_, doorNum_;//房间和门的总数目
}Maze, *PMaze, **PPMaze;

typedef struct tagRoomSet
{
	PPRoom rooms_;
	int count_;
}RoomSet,*PRoomSet,**PPRoomSet;

typedef struct tagDoorSet
{
	PPDoor doors_;
	int count_;
}DoorSet, *PDoorSet, **PPDoorSet;


#ifdef __cplusplus
extern "C"
{
#endif
#ifdef _USRDLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif


	DLLEXPORT NONE AllocMaze(DIMLENGTH _dimNum, DIMCONTENT _dimIn, PMaze _outMaze);
	DLLEXPORT DIMLENGTH RegularDim(DIMLENGTH _dimNum, DIMCONTENT _dimIn, PDIMCONTENT _dimOut);
	DLLEXPORT NONE RegularDimForMaze(PMaze _maze, PDIMCONTENT _dim, DIMLENGTH _inputDimNum);
	DLLEXPORT NONE InitializeDoor(PPDoor _door);
	DLLEXPORT NONE CopyDim(DIMCONTENT _DimFrom, PDIMCONTENT _DimTo, DIMLENGTH _DimNum);
	DLLEXPORT PRoom FindRoom(DIMLENGTH _dimNum, DIMCONTENT _dim, PMaze _maze);
	DLLEXPORT NONE RegularDimForMaze(PMaze _maze, PDIMCONTENT _dim, DIMLENGTH _inputDimNum);
	DLLEXPORT BOOL RegularDim(DIMLENGTH _dimNum, DIMCONTENT _dimIn, PDIMCONTENT _dimOut);
	DLLEXPORT NONE FreeRoom(PRoom _room);
	DLLEXPORT NONE FreeMaze(PMaze _maze);
	DLLEXPORT PDoorSet GetRoomDoorsEx(PRoom _room, DIMLENGTH _dim);
	DLLEXPORT NONE FreeDoorSet(PDoorSet _doorSet);
	DLLEXPORT NONE CopyDim(DIMCONTENT _DimFrom, PDIMCONTENT _DimTo, DIMLENGTH _DimNum);
	DLLEXPORT DIMCONTENT CopyDimEx(DIMCONTENT _Dim, DIMLENGTH _DimNum);
	DLLEXPORT NONE AllocDoorSet(PDoorSet _doors, int _doorNum);
	DLLEXPORT NONE InitializeDoorSet(PDoorSet _ret);
	DLLEXPORT STRING DimValue(PDim _dim);
	DLLEXPORT STRING DimValue2(DIMCONTENT, DIMLENGTH);
	DLLEXPORT NONE DumpRoomSet(PPRoom _rooms, int _size);
	DLLEXPORT NONE DumpDoorSet(PPDoor _doors, int _size);
	DLLEXPORT BOOL RoomInitialized(PRoom _room);
	DLLEXPORT NONE MarkDoorUsed(PDoor _door);
	DLLEXPORT NONE MarkDoorEdge(PDoor _door);
	DLLEXPORT NONE MarkRoomDestroyed(PRoom _room);
	DLLEXPORT NONE MarkDoorDestroyed(PDoor _door);
	DLLEXPORT NONE DumpRoom(PRoom _room);
	DLLEXPORT NONE DumpDoor(PDoor _door, PRoom _roomIgnore);
	DLLEXPORT BOOL isDoorCreated(PDoor _door);
	DLLEXPORT DIMCONTENT GenerateDefaultDim(PMaze _maze);
#undef DLLEXPORT
#ifdef __cplusplus
}
#endif
#endif