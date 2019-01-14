#pragma once

#ifndef _PATH_DEFINE_H_
#define _PATH_DEFINE_H_

#include "TypeDef.h"

#ifndef _MAZE_DEFINE_H_
#include "Maze.h"
#endif

typedef struct tagPathFlag
{
	int MainWayConfirmed_ : 1;
}PathFlag;

typedef union tagUPathFlag
{
	struct tagPathFlag flags_;
	unsigned long value_;
}UPathFlag;

typedef struct tagPathNodeRoom
{
	PPRoom roomInPath_,roomOutPath_;
	int inSize_, outSize_;
	int maxSize_;
}PathNodeRoom,*PPathNodeRoom,**PPPathNodeRoom;

typedef struct tagPath
{
	PRoom roomStart_, roomEnd_;
	struct tagPathNode* firstNode_;
	struct tagPathNodeRoom pathNodeRoom_;
	UPathFlag flag_;
	Maze* maze_;
}Path,*PPath,**PPPath;

typedef struct tagPathNode
{
	PRoom room_;//当前房间
	PDoor enterDoor_;//进入此房间所用的门
	struct tagPathNode* MainWay_, *SubWay_, *Parent_;//下一步，主路、支路、父节点
	PPath path_;
	unsigned long flags_;
}PathNode,*PPathNode,**PPPathNode;

typedef enum tagInPathOrNot
{
	ION_UNKNOWN = -1,
	ION_INPATH = 1,
	ION_OUTPATH = 0,
	ION_ILLEGAL = -2
}InPathOrNot;

typedef enum tagPathStatus
{
	SS_PATH_MAINPATH_CONFIRMED = 0x08000000
}PathStatus;

typedef struct tagNearNeighborInPath//在路径上的邻居集合
{
	PRoom room_;
	PPathNode pathNode_;
	PDoor linkDoor_;
}NearNeighborInPath,*PNearNeighborInPath;

typedef struct tagRoomWithNeighborInPath//房间和它在路径上的邻居集合
{
	struct tagNearNeighborInPath* nearInPathRooms_;
	int nearRoomNumbers_;
	PRoom Room_;
}RoomWithNeighborInPath,*PRoomWithNeighborInPath;

#define CONFIRMED_MAIN_ROAD 0x40000000
#define HAD_CONFIRMED_MAIN_ROAD(a) ((((a)&CONFIRMED_MAIN_ROAD)==0)?0:1)
#define FLAG_CONFIRM_MAIN_ROAD(a) (a|=CONFIRMED_MAIN_ROAD)

#define DLLEXPORTS __declspec(dllexport)

DLLEXPORTS void FreePath(PPPathNode _path);
DLLEXPORTS PRoomSet AllocRoomSet();
DLLEXPORTS void FreeRoomSet(PPRoomSet);
DLLEXPORTS DoorSet* CreateDoorSet();
DLLEXPORTS void FreeDoorSet(DoorSet*);
DLLEXPORTS PRoomSet GetNeighborRooms(PRoom _room, int _dim);
DLLEXPORTS DoorSet* GetRoomDoors(PRoom _room, PMaze _maze);
DLLEXPORTS PRoomSet GetNeighborNotInPath(PRoom _room, PMaze _maze, PPath _path);
DLLEXPORTS PRoom RandomSelectRoom(PRoomSet _rooms);
DLLEXPORTS void MoveRoomInPathRoom(PPathNodeRoom _pr, int _pos);
DLLEXPORTS PPathNode AllocPathNode(PRoom _room, PPath _path);
DLLEXPORTS PPath AllocPath();
DLLEXPORTS void PreparePath(PPPath _path, PMaze _maze, DIMCONTENT _dimEntry, DIMLENGTH _dimEntryLength, DIMCONTENT _dimExit, DIMLENGTH _dimExitLength);
DLLEXPORTS void FreePath(PPPathNode _path);
DLLEXPORTS InPathOrNot RoomInPath(PPath _path, PRoom _room);
DLLEXPORTS void DumpPathRoom(PPathNodeRoom _pnr);
DLLEXPORTS DoorSet* GetRoomDoorsEx(PRoom _room, int _dim);
DLLEXPORTS PDoor GetDoorBetweenRooms(PRoom _roomFrom, PRoom _roomTo);
DLLEXPORTS int GetAllDoorsToRoomsNotInPath(PRoom _room, PDoorSet _ret, PPath _path);
DLLEXPORTS PPath BuildPath(PMaze _maze, PDim _startRoom, PDim _endRoom);
DLLEXPORTS void DumpPath(PPathNode _path);
DLLEXPORTS PRoomWithNeighborInPath GetRoomNeighborInPath(PRoom _room, PPath _path);
DLLEXPORTS PRoomWithNeighborInPath AllocRoomWithNeighborInPath(PRoom _room);
DLLEXPORTS void FreeRoomWithNeighborInPath(PRoomWithNeighborInPath _roomWNIP);
DLLEXPORTS void AddNeighborToNNIP(PRoomWithNeighborInPath _roomWNIP,PRoom _neighbor,PPath _path);

#undef DLLEXPORTS

#endif
