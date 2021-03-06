// debugMaze.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#pragma once


#include <gl/glew.h>
#include <gl/glut.h>

#include "Batch.h"
#include "ShaderManager.h"

#define DLLEXPORT __declspec(dllimport)

extern "C" {
	typedef struct tagDim
	{
		int* values_;
		int length;
	}Dim;

	typedef struct tagRoom
	{
		unsigned long flags_;
		struct tagDoor** doors_;
		int dimNum_;//维度的数目
		int* dim_;//房间自身的维度，用于调试，为节省空间可以去除
	}Room;

	typedef struct tagDoor
	{
		unsigned long flags_;
		Room* linkedRooms_[2];
		unsigned char isClosed_;
	}Door;

	typedef struct tagMaze
	{
		Room* rooms_;
		Door** doors_;
		int* factorEveryDim_;//每个维度的计算因子，为方便查找定位
		int* dims_;
		int dimNum_;//维度的数目
		int roomNum_, doorNum_;//房间和门的总数目
	}Maze;

	typedef struct tagPathFlag
	{
		int MainWayConfirmed_ : 1;
	}PathFlag;

	typedef union tagUPathFlag
	{
		PathFlag flags_;
		unsigned long value_;
	}UPathFlag;

	typedef struct tagPathNodeRoom
	{
		Room** roomInPath_, ** roomOutPath_;
		int inSize_, outSize_;
		int maxSize_;
	}PathNodeRoom;

	typedef struct tagPath
	{
		Room* roomStart_, *roomEnd_;
		struct tagPathNode* firstNode_;
		struct tagPathNodeRoom pathNodeRoom_;
		UPathFlag flag_;
		Maze* maze_;
	}Path;

	typedef struct tagPathNode
	{
		Room* room_;//当前房间
		Door* enterDoor_;//进入此房间所用的门
		struct tagPathNode* MainWay_, *SubWay_, *Parent_;//下一步，主路、支路、父节点
		Path* path_;
		unsigned long flags_;
	}PathNode;

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

	typedef struct tagRoomSet
	{
		Room** rooms_;
		int count_;
	}RoomSet,*PRoomSet;

	typedef struct tagDoorSet
	{
		Door** doors_;
		int count_;
	}DoorSet;

	typedef void NONE;
	typedef int BOOL;

DLLEXPORT NONE AllocMaze(int _dimNum, int* _dimIn, Maze* _outMaze);
DLLEXPORT BOOL RegularDim(int _dimNum, int* _dimIn, int** _dimOut);
DLLEXPORT NONE RegularDimForMaze(Maze* _maze, int** _dim, int _inputDimNum);
DLLEXPORT NONE InitializeDoor(Door** _door);
DLLEXPORT NONE CopyDim(int* _DimFrom, int** _DimTo, int _DimNum);
DLLEXPORT Room* FindRoom(int _dimNum, int* _dim, Maze* _maze);
DLLEXPORT void FreeRoom(Room* _room);
DLLEXPORT void FreeMaze(Maze* _maze);
DLLEXPORT void FreePath(PathNode** _path);
DLLEXPORT RoomSet* AllocRoomSet();
DLLEXPORT void FreeRoomSet(RoomSet**);
DLLEXPORT DoorSet* CreateDoorSet();
DLLEXPORT void FreeDoorSet(DoorSet*);
DLLEXPORT RoomSet* GetNeighborRooms(Room* _room, int _dim);
DLLEXPORT DoorSet* GetRoomDoors(Room* _room, Maze* _maze);
DLLEXPORT RoomSet* GetNeighborNotInPath(Room* _room, Maze* _maze, Path* _path);
DLLEXPORT Room* RandomSelectRoom(RoomSet* _rooms);
DLLEXPORT int* GenerateDefaultDim(Maze* _maze);
DLLEXPORT void MoveRoomInPathRoom(PathNodeRoom* _pr, int _pos);
DLLEXPORT PathNode* AllocPathNode(Room* _room, Path* _path);
DLLEXPORT Path* AllocPath();
DLLEXPORT void PreparePath(Path** _path, Maze* _maze, int* _dimEntry, int _dimEntryLength, int* _dimExit, int _dimExitLength);
DLLEXPORT void FreePath(PathNode** _path);
DLLEXPORT InPathOrNot RoomInPath(Path* _path, Room* _room);
DLLEXPORT void DumpPathRoom(PathNodeRoom* _pnr);
DLLEXPORT void DumpRoom(Room* _room);
DLLEXPORT void DumpDoor(Door* _door, Room* _roomIgnore);
DLLEXPORT DoorSet* GetRoomDoorsEx(Room* _room, int _dim);
DLLEXPORT Door* GetDoorBetweenRooms(Room* _roomFrom, Room* _roomTo);
DLLEXPORT int GetAllDoorsToRoomsNotInPath(Room* _room, DoorSet* _ret, Path* _path);
DLLEXPORT Path* BuildPath(Maze* _maze, Dim* _startRoom, Dim* _endRoom);
}

#undef DLLEXPORT

void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);
}

void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BITS | GL_STENCIL_BITS);

	glutSwapBuffers();
}

int main(int argc,char* argv[])
{
	//const int DIMLEN = 3;
	//const int DIM[DIMLEN] = { 4,4,4 };

	//int dim[DIMLEN] = { DIM[0],DIM[1],DIM[2] };// , DIM[2], DIM[3]
	//Maze maze = { NULL,NULL,NULL,NULL,0,0,0 };
	//AllocMaze(DIMLEN, dim, &maze);

	//Path *path_p=NULL;
	//PreparePath(&path_p, &maze, NULL, 0, NULL, 0);
	///*for (int i = 0; i < DIM[0]; i++)
	//	for (int j = 0; j < DIM[1]; j++)
	//		for (int k = 0; k < DIM[2]; k++)
	//			for (int l = 0; l < DIM[3]; l++)
	//			{
	//				int* dim2 = (int*)malloc(DIMLEN * sizeof(int));
	//				dim2[0] = i;
	//				dim2[1] = j;
	//				dim2[2] = k;
	//				dim2[3] = l;
	//				Room* room = FindRoom(DIMLEN, dim2, &maze);
	//				int* dim3 = room->dim_;
	//				printf("(%d)%d,%d,%d,%d=> %d,%d,%d,%d\r\n",
	//					room - maze.rooms_,
	//					dim2[0], dim2[1], dim2[2], dim2[3],
	//					dim3[0], dim3[1], dim3[2], dim3[3]);
	//				free(dim2);
	//			}*/
	//
	////DumpPathRoom(&path_p->pathNodeRoom_);
	//Dim start, end;
	//start.length = 3;
	//end.length = 3;
	//start.values_ = (int*)malloc(sizeof(int) * 3);
	//end.values_ = (int*)malloc(sizeof(int) * 3);
	//start.values_[0] = 0;
	//start.values_[1] = 0;
	//start.values_[2] = 0;
	//end.values_[0] = 3;
	//end.values_[1] = 3;
	//end.values_[2] = 3;
	//BuildPath(&maze, &start, &end);	
	
	glutInit(&argc, argv);
	glutInitWindowSize(1024, 768);
	glutCreateWindow("");
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);

	glutMainLoop();
	return 0;
}

