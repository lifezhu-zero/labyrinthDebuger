#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <memory.h>
#include <stdio.h>

#include "Path.h"
#include "PointerSet.h"
#include "Rand.h"
#include "OLinkLine.h"
static PPathNode InsertPathNode(PPathNode _currentNode, PPathNode _nodeInsert, PPath _path);

void MoveRoomInPathRoomByRoom(PPathNodeRoom _pr, PRoom _room)
{
	int pos = -1;
	if (_pr != NULL)
	{
		for (int i = 0; i < _pr->outSize_; i++)
			if (_pr->roomOutPath_[i] == _room)
			{
				pos = i;
				break;
			}
			else if (_pr->roomOutPath_[i] > _room)
				break;
	}

	if (pos >= 0)
		MoveRoomInPathRoom(_pr, pos);

	return;
}

//将一个房间从不在路径树中的状态移到已在路径树的状态上
//_pr 路径对象
//_pos 该房间在路径树不在路径树数组中的位置
void MoveRoomInPathRoom(PPathNodeRoom _pr, int _pos)
{
	int i;
	PPRoom buffer;
	int size;
	assert(_pos >= 0 && _pos < _pr->outSize_);

	PRoom room = _pr->roomOutPath_[_pos];
	//移出不在路径树上数组
	size = (_pr->outSize_ - _pos - 1) * sizeof(PRoom);
	if (size > 0)
	{
		buffer = (PPRoom)malloc(size);
		memcpy(buffer, _pr->roomOutPath_ + _pos + 1, size);
		memcpy(_pr->roomOutPath_ + _pos, buffer, size);//后面的前移
		free(buffer);
	}

	_pr->outSize_--;

	PRoom room0 = NULL;
	//找到已在路径树上的第一个空白位置或者第一个比要加入的指针大的位置
	for (i = 0; i < _pr->inSize_; i++)
	{
		room0 = _pr->roomInPath_[i];
		if (room0 == NULL || room0 > room)
			break;
	}

	if (i < _pr->inSize_)
	{
		if (room0 == NULL)
			_pr->roomInPath_[i] = room;
		else//插入，所有已有的后移
		{
			assert(i <= _pr->inSize_ - 1);
			size = (_pr->inSize_ - i) * sizeof(PRoom);

			if (size > 0)
			{
				buffer = (PPRoom)malloc(size);//需要一个缓冲
				memcpy(buffer, _pr->roomInPath_ + i, size);
				memcpy(_pr->roomInPath_ + i + 1, buffer, size);
				free(buffer);
			}

			_pr->roomInPath_[i] = room;
			_pr->inSize_++;
		}
	}
	else if (_pr->inSize_ < _pr->maxSize_)//直接在尾部添加
	{
		_pr->roomInPath_[_pr->inSize_] = room;
		_pr->inSize_++;
	}

	DumpRoomSet(_pr->roomInPath_, _pr->inSize_);
	DumpRoomSet(_pr->roomOutPath_, _pr->outSize_);
}

//用指定的房间和路径初始化分配一个路径节点
PPathNode AllocPathNode(PRoom _room, PPath _path)
{
	PPathNode ret = (PPathNode)malloc(sizeof(PathNode));
	ret->enterDoor_ = NULL;
	ret->MainWay_ = NULL;
	ret->SubWay_ = NULL;
	ret->Parent_ = NULL;
	ret->room_ = _room;
	ret->path_ = _path;
	ret->flags_ = 0;
	return ret;
}

//初始化路径
PPath AllocPath()
{
	PPath ret = (PPath)malloc(sizeof(Path));
	ret->firstNode_ = NULL;
	ret->flag_.value_ = 0;
	ret->pathNodeRoom_.roomInPath_ = NULL;
	ret->pathNodeRoom_.roomOutPath_ = NULL;
	ret->pathNodeRoom_.inSize_ = 0;
	ret->pathNodeRoom_.outSize_ = 0;
	ret->maze_ = NULL;
	ret->roomStart_ = NULL;
	ret->roomEnd_ = NULL;
	return ret;
}

//准备路径建立，初始化路径变量，路径的入口节点
//_path 返回新创建的路径入口节点
//_maze 需要生成路径的迷宫对象
//_dimEntry,_dimEntryLength 入口的房间坐标，缺省使用(0,0,...)作为入口
//_dimExit,_dimExitLength 出口的房间坐标，缺省使用所有维度最大坐标作为出口
void PreparePath(PPPath _path, PMaze _maze, DIMCONTENT _dimEntry, DIMLENGTH _dimEntryLength, DIMCONTENT _dimExit, DIMLENGTH _dimExitLength)
{
	PRoom room = FindRoom(_dimEntryLength, _dimEntry, _maze);//首先找到入口房间
	int size;//路径房间的数目
	int pos;//初始化后，入口房间在路径房间上的位置

	if (room == NULL)//入口房间为空，使用全零坐标房间作为入口房间
	{
		DIMCONTENT dim = GenerateDefaultDim(_maze);
		room = FindRoom(_maze->dimNum_, dim, _maze);
		free(dim);
	}

	*_path = AllocPath();//分配路径变量
	AllocPathNode(room, *_path);
	PPath path = *_path;

	path->firstNode_ = AllocPathNode(room, *_path);//使用入口房间创建入口节点
	path->maze_ = _maze;//指定路径使用的迷宫

	PPathNode pathNode = path->firstNode_;
	//初始化路径节点包含房间
	PPathNodeRoom pathNodeRoom = &path->pathNodeRoom_;

	size = sizeof(PRoom)*_maze->roomNum_;
	pathNodeRoom->roomInPath_ = (PPRoom)malloc(size);
	memset(pathNodeRoom->roomInPath_, 0, size);
	pathNodeRoom->roomOutPath_ = (PPRoom)malloc(size);
	memset(pathNodeRoom->roomOutPath_, 0, size);
	pathNodeRoom->maxSize_ = _maze->roomNum_;

	pathNodeRoom->inSize_ = 0;
	pathNodeRoom->outSize_ = _maze->roomNum_;
	path->roomStart_ = room;

	//所有房间在初始化时均归入不在路径上的房间
	for (int i = 0; i < _maze->roomNum_; i++)
	{
		pathNodeRoom->roomOutPath_[i] =
			&_maze->rooms_[i];
		if (path->roomStart_ == &_maze->rooms_[i])
			pos = i;//记录入口房间的位置
	}

	//首先将入口房间移到已在路径上房间中
	MoveRoomInPathRoom(pathNodeRoom, pos);

	//查找出口房间
	room = FindRoom(_dimExitLength, _dimExit, _maze);

	if (room == NULL)//如果出口房间不存在，使用各维最大维度房间作为出口房间
	{
		DIMCONTENT dim;
		dim = CopyDimEx(_maze->dims_, _maze->dimNum_);
		room = FindRoom(_maze->dimNum_, dim, _maze);
		free(dim);
	}

	path->roomEnd_ = room;//标记出口房间
}

void FreePath(PPPathNode _path)
{
	PPathNode path = *_path;
	//PPathNodeRoom pathNodeRoom = path->pathRoom_;
	//path->pathRoom_->outSize_ = 0;
	//path->pathRoom_->inSize_ = 0;
	//free(path->pathRoom_->roomInPath_);
	//free(path->pathRoom_->roomOutPath_);
	//free(path->pathRoom_);
	//path->pathRoom_ = NULL;

	free(path);
	*_path = NULL;
}

//判断房间是否处在路径树上
InPathOrNot RoomInPath(PPath _path, PRoom _room)
{
	PPathNodeRoom pr = &_path->pathNodeRoom_;
	InPathOrNot ret = ION_UNKNOWN;
	int i;

	//以下简单逐个查找数组中内容，数组已经排序，可以改成二分法
	for (i = 0; i < pr->inSize_; i++)//在路径已包含房间数组中查找指定房间
	{
		if (pr->roomInPath_[i] == _room)//查到，房间在路径上
		{
			ret = ION_INPATH;
			break;
		}
		else if (pr->roomInPath_[i] > _room)//查不到
			break;
	}

	if (ret == ION_UNKNOWN)//查不到，检查不在路径上房间数组
	{
		for (i = 0; i < pr->outSize_; i++)
		{
			if (pr->roomOutPath_[i] == _room)//查到，房间不在路径上
			{
				ret = ION_OUTPATH;
				break;
			}
			else if (pr->roomOutPath_[i] > _room)//查不到，存疑
			{
				ret = ION_ILLEGAL;
				break;
			}
		}
	}
	return ret;
}


PRoom RandomSelectRoom(PRoomSet _rooms)
{
	PRoom ret = NULL;
	if (_rooms != NULL && _rooms->count_ > 0)
		ret = _rooms->rooms_[Random(0, _rooms->count_)];

	return ret;
}

//在当前路径所在的房间随机选择一个不在路径且门是开的邻居作为当前房间
//							如果当前路径的主路径为空，作为主路径加入新的路径，
//								否则如果支路径为空，作为支路径加入当前路径，
//								否则，将当前路径的房间拷贝生成一个路径作为当前路径，
//										其父路径指向之前的当前路径，新的路径作为新的当前路径的主路径加入路径树
//如果不存在这样的房间则回溯到上一个房间（可能经历多个路径），并且将回溯的路径是其父路径的主路径全部交换到支路
//如果无法回溯（父路径为空），从不在路径上的房间中随机选择一个房间作为当前房间，
//							在该房间的邻居中随机选择一个路径内房间，并打通两者之间的房门？
//

//判断一个门是否是这个房间的门，而且门连接着另一个房间
static int DoorInRoom(PRoom _room, PDoor _door)
{
	assert(_room != NULL);
	assert(_door != NULL);
	assert(_room->Doors_ != NULL);
	assert(_room->dimNum_ > 0);
	assert(_room->dim_ != NULL);

	for (int i = 0; i < _room->dimNum_ * 2; i++)
	{
		PPDoor door_p = (PPDoor)GetPointerPS(_room->Doors_->doors_, _room->Doors_->count_, i);
		assert(door_p != NULL);
		if (*door_p == _door)
			if (
				(_door->linkedRooms_[0] == _room && _door->linkedRooms_[1] != NULL)
				||
				(_door->linkedRooms_[1] == _room && _door->linkedRooms_[0] != NULL)
				)
				return 1;
			else
				return 0;
	}
	return 0;
}

//创建连接两个房间的路径，并将它加入路径树
PPathNode CreatePathBetweenRoom(PPPathNode _currentNode, PDoor _selectDoor)
{
	assert(_currentNode != NULL);
	assert(*_currentNode != NULL);
	assert(_selectDoor != NULL);

	PPathNode node = *_currentNode;
	PPPathNode fillNode = NULL;
	PPathNode insertNode = AllocPathNode(node->room_, node->path_);

	assert(DoorInRoom(node->room_, _selectDoor));

	if (node->MainWay_ == NULL)
		fillNode = &node->MainWay_;
	else if (node->SubWay_ == NULL)
		fillNode = &node->SubWay_;
	else
		return CreatePathBetweenRoom(fillNode, _selectDoor);

	return NULL;
}

//判断包含_path的路径是否已经确定主路径
static inline int isMainWayConfirmed(PPathNode _pathNode)
{
	assert(_pathNode != NULL);
	PPath path = _pathNode->path_;


	return (path->flag_.flags_.MainWayConfirmed_ != 0);
}

//找到或者创建一个可以赋与子路径的路径节点
PPPathNode FindOrCreateFirstCanAssignPath(PPPathNode _currentPath)
{
	PPathNode currentPath = *_currentPath;
	if (currentPath->MainWay_ == NULL)
		return &currentPath->MainWay_;
	else if (currentPath->SubWay_ == NULL)
		return &currentPath->SubWay_;
	else
	{
		PRoom room = currentPath->room_;
		assert(room != NULL);
		//todo
	}

	return NULL;
}

//将当前路径节点指定的房间通过指定的门将其某个邻居链接到路径里
PPathNode MakePath(PPPathNode _currentPath, PDoor _door, int _dimNum)
{
	PPathNode currentPath = *_currentPath;
	PRoom room = currentPath->room_;

	assert(room != NULL || room->Doors_ == NULL);
	int i;
	for (i = 0; i < _dimNum * 2; i++)//首先检查该门属于此房间
	{
		PPDoor door_p = (PPDoor)GetPointerPS(room->Doors_->doors_, room->Doors_->count_, i);
		if (*door_p == _door)
			break;
	}

	if (i < _dimNum * 2)//是此节点对应房间的门
	{
		PPPathNode assignPath = NULL;
		if (currentPath->MainWay_ == NULL)
		{
			assignPath = &currentPath->MainWay_;
		}
		else if (currentPath->SubWay_ == NULL)
		{
			assignPath = &currentPath->SubWay_;
		}
		//ToDo 需要把房间从路径的未在节点数组中转移到已在节点数组中
	}
	return NULL;
}


PPathNode BackwardToRoomRoot(PPathNode _node);
PPathNode FindPositionInsertPathNode(PPPathNode _nodes, int _nodesNum, int _dimNum);

static int DimNumberOfPathNode(PPathNode _pathNode)
{
	assert(_pathNode != NULL);
	assert(_pathNode->path_ != NULL);
	assert(_pathNode->path_->maze_ != NULL);
	return _pathNode->path_->maze_->dimNum_;
}

//回溯一个节点，如果此时整体路径的主路径尚未确立，或者当前节点的父节点不是主路径上的节点，（则交换父节点的主支节点的位置）
//							2018-4-24 仅交换主支节点并不严谨，（需要将当前节点移到最后，空出主节点位置）
//							2018-4-26 完全可以在主路径确立以后重构主路径结构，以此简化路径建立，也不用担心破坏单个房间的平衡二叉树的结构
//										这样在回溯时就不用考虑主从分路的问题
//						（修改上述操作为 如果该节点在父节点中的分路位置是分路节点，则找到该房间的插入位置，并将此节点插入到该插入位置上，原主分路置NULL)
//返回值为回溯所达到的节点，必须与产生回溯的节点房间不同，调用函数重新选择房间
//典型的回溯操作是当前节点的房间已经无法创建新的路径，因此需要返回此房间的入口房间继续试图创建路径
//注：仅回溯，找到其路径上第一个有不在路径上的相邻房间即可，返回NULL表示路径生成结束
PPathNode BackwardPath(PPathNode _currentPath)
{
	PPathNode RoomEntryNode = BackwardToRoomRoot(_currentPath);//首先找到当前房间的入口节点（即该节点的父节点并非当前房间）
	if (RoomEntryNode != NULL)//如果根节点存在
	{
		//获取入口节点
		PPathNode parentNode = RoomEntryNode->Parent_;

		if (parentNode != NULL)
		{
			//获取入口房间
			PRoom room = parentNode->room_;

			//入口房间如果有不在Path上的邻居房间，则返回该节点，回溯终止，否则递归回溯，直到无法回溯，返回空值
			//printf("Back throw room:");
			//DumpRoom(room);

			//检查当前房间的邻居，是否有不在Path上的情况
			PRoomSet neighbors = GetNeighborRooms(room, DimNumberOfPathNode(_currentPath));//获取当前房间的邻居

			//DumpRoomSet(neighbors->rooms_,neighbors->count_);
			if (neighbors != NULL && neighbors->count_ > 0)
			{
				PPath path = _currentPath->path_;

				for (int i = 0; i < neighbors->count_; i++)
				{
					InPathOrNot ipon = RoomInPath(path, neighbors->rooms_[i]);
					if (ipon == ION_INPATH)
						continue;
					else if (ipon == ION_OUTPATH)
					{
						FreeRoomSet(&neighbors);
						return parentNode;//当前房间有存在邻居不在路径上，返回当前房间
					}
				}

				FreeRoomSet(&neighbors);
				//递归查找当前房间的上一层
				return BackwardPath(parentNode);
			}
			else
			{
				printf("Neighbor Rooms is NULL\r\n");
			}
		}
		else
		{
			printf("Entry Node is NULL\r\n");
		}
	}
	else
	{
		printf("Self Root Node is NULL\r\n");
	}
	//如果找不到入口节点，说明无法回溯
	return NULL;
}

//在节点的主/次分路上插入一个指定房间的节点，返回插入的节点
//如果当前节点的主分路不为空则将新节点赋给次分路，否则赋给主分路
//10-18 有问题，主次分路都非空时依然可以插入，在寻找可插入节点时必须返回插入位置是主分路还是次分路？进一步分析：不需要，首选分路指向房间和当前房间不一致的，如果两条分路都不一致则插入主分路，当然，插入后原分路指向的节点如果非空必须修改入口节点
//_currentNode 已经判定可以插入新节点的节点
//_room 新插入节点的房间
PPathNode InsertPathNode(PPathNode _currentNode, PPathNode _nodeInsert, PPath _path)
{
	assert(_currentNode != NULL);

	//添加节点的节点位置 被添加到路径中的节点
	PPPathNode nodeTobeInsert = NULL;
	if (_currentNode->MainWay_ == NULL)
		nodeTobeInsert = &_currentNode->MainWay_;
	else if (_currentNode->SubWay_ == NULL)
		nodeTobeInsert = &_currentNode->SubWay_;
	else if (_currentNode->SubWay_->room_ != _currentNode->room_)
		nodeTobeInsert = &_currentNode->SubWay_;
	else if (_currentNode->MainWay_->room_ != _currentNode->room_)
		nodeTobeInsert = &_currentNode->MainWay_;

	//??????????????????????????????
	_nodeInsert->MainWay_ = *nodeTobeInsert;//旧的子节点挂到插入的节点的主分路上 即使原值为空
	if (nodeTobeInsert != NULL && *nodeTobeInsert != NULL)
	{
		(*nodeTobeInsert)->Parent_ = _nodeInsert;
		//printf("Node To Be Insert:");
		//DumpPath(*nodeTobeInsert);
		//printf("&&&&&&&&&&&&&&&&&&&&&&\r\n");
	}

	*nodeTobeInsert = _nodeInsert;//插入的节点挂到插入的节点位置

	_nodeInsert->Parent_ = _currentNode;
	//printf("Node Inserted:");
	//DumpPath(_nodeInsert);
	//printf("&&&&&&&&&&&&&&&&&&&&&&\r\n");

	return _nodeInsert;//返回被插入的节点
}

//在已知房间相同的路径节点里找到第一个能够存放新的从该房间出发的路径节点的指针位置
//_nodes 必须是同一房间相关的路径节点在同一层次上的兄弟节点
//_nodesNum 在_nodes里的节点数目
//_dimNum 维度的数目
//_isSubWay 新添加的位置是否是返回节点的支路节点 //10-2 去掉? 去掉了
//返回值 找到的能添加或者插入的节点的父节点
//此函数是递归的，如果在当前节点找不到位置，会收集其所有同层次的一代子节点递归查找
PPathNode FindPositionInsertPathNode(PPPathNode _nodes, int _nodesNum, int _dimNum)
{
	int i, size;
	PPPathNode subNodes = NULL;//用来存放子一层的节点
	PPPathNode currentSubNodesP = NULL;//用于遍历的当前子一层节点存储的位置
	PPathNode ret = NULL;
	PRoom room = NULL;
	PPathNode node0 = _nodes[0];//需要中间插入复制节点的位置

	size = _dimNum * 2;
	subNodes = (PPPathNode)malloc(sizeof(PPathNode)*size);//预分配最大可能空间，每个门一个
	memset(subNodes, 0, sizeof(PPathNode)*size);//初始化子一层节点所有指针为空

	currentSubNodesP = subNodes;
	//对于单个房间来说，可以有多个路径节点，且这些节点以平衡二叉树的结构存储
	//遍历所有存储在_nodes里的节点,检查其子节点
	for (i = 0; i < _nodesNum; i++)
	{
		PPathNode node = _nodes[i];
		if (node != NULL)
		{
			if (room == NULL)
				room = node->room_;

			PPathNode nodeSubMain = node->MainWay_;
			PPathNode nodeSubSub = node->SubWay_;

			if (nodeSubMain == NULL)//空的位置->添加 
			{
				ret = node;
				break;
			}
			else if (nodeSubMain->room_ != room && nodeSubSub != NULL)//下层是其它房间->插入
			{
				//	PPathNode newCloneNode;//将父节点拷贝一份作为插入节点，并插入到
				//	newCloneNode = AllocPathNode(room, node->path_);

				//	newCloneNode->enterDoor_ = node->enterDoor_;
				//	newCloneNode->Parent_ = node;
				//	newCloneNode->MainWay_ = node->MainWay_;
				//	node->MainWay_ = newCloneNode;
				//	ret = newCloneNode;
				node0 = node;
				break;
			}
			else if (nodeSubMain->room_ == node->room_)//记录该子节点到subNodes
			{
				if (nodeSubMain)
					*currentSubNodesP++ = node->MainWay_;
			}


			if (nodeSubSub == NULL)  //空的位置->添加 
			{
				ret = node;
				break;
			}
			else if (nodeSubSub->room_ != room)//下层是其它房间->插入
			{
				//	PPathNode newCloneNode;//将父节点拷贝一份作为插入节点，并插入到
				//	newCloneNode = AllocPathNode(room, node->path_);

				//	newCloneNode->enterDoor_ = node->enterDoor_;
				//	newCloneNode->Parent_ = node;
				//	newCloneNode->MainWay_ = node->MainWay_;
				//	node->SubWay_ = newCloneNode;
				//	ret = newCloneNode;
				node0 = node;
				break;
			}
			else if (nodeSubSub->room_ == node->room_)//记录该子节点到subNodes
			{
				if (nodeSubSub)
					*currentSubNodesP++ = node->SubWay_;
			}
		}
	}

	if (ret == NULL)
	{
		//运行到这里意味着当前层找不到可以添加子节点的位置，递归查找
			//应该不会出现这种情况 11-30 当一个房间打开的门超过两个就会出现这种情况
		if (currentSubNodesP == subNodes)//一种情况是上层是完全二叉树，没有子节点提供递归查找
		{
			assert(node0 != NULL);

			//新创建一个同一房间的节点插入到第一个子节点下面，并将原来的节点挂到这个节点的分路节点上，并返回这个节点，以它的主路节点作为挂入点
			PPathNode newNode = AllocPathNode(room, NULL);
			newNode->enterDoor_ = node0->enterDoor_;
			newNode->Parent_ = node0;
			if (node0->MainWay_ == NULL || node0->MainWay_->room_ != room)
			{
				PPathNode node1 = node0->MainWay_;
				node0->MainWay_ = newNode;
				newNode->MainWay_ = node1;
				ret = newNode;
			}
			else if (node0->SubWay_ == NULL || node0->SubWay_->room_ != room)
			{
				PPathNode node1 = node0->SubWay_;
				node0->SubWay_ = newNode;
				newNode->MainWay_ = node1;
				ret = newNode;
			}
		}
		else
			ret = FindPositionInsertPathNode(subNodes, (int)(currentSubNodesP - subNodes), _dimNum);
	}

	free(subNodes);
	return ret;
}

void DumpPathRoom(PPathNodeRoom _pnr)
{
	if (_pnr != NULL)
	{
		int i, size;
		size = _pnr->inSize_;
		printf("Rooms in path %d\r\n", size);
		for (i = 0; i < size; i++)
			DumpRoom(_pnr->roomInPath_[i]);
		size = _pnr->outSize_;
		printf("Rooms out of path %d\r\n", size);
		for (i = 0; i < size; i++)
			DumpRoom(_pnr->roomOutPath_[i]);
	}
	else
	{
		printf("PathRoom is NULL\r\n");
	}
}

//向上找到和指定节点同一房间的根节点
PPathNode BackwardToRoomRoot(PPathNode _node)
{
	while (_node->Parent_ != NULL && _node->Parent_->room_ == _node->room_)
	{
		//if (_node->Parent_ != NULL)//当前房间的根节点不能是整棵树的根节点
		_node = _node->Parent_;
	}
	return _node;
}

PDoor RandomSelectDoor(PDoorSet _doors)
{
	return _doors->doors_[Random(0, _doors->count_)];
}

//将两个节点中的房间通过门连接起来，并修改Path的两个数组，最后放到路径树上（在当前节点上寻找能够插入的位置）
void LinkCurrentNodeToNewRoomNode(PPPathNode _currentNode, PPathNode _newNode)
{
	assert(_currentNode != NULL);
	assert(*_currentNode != NULL);

	PPathNode currentNode = *_currentNode;
	//先找出连接两个房间的门
	PDoor door = GetDoorBetweenRooms(currentNode->room_, _newNode->room_);
	if (door != NULL)
	{
		//找出当前节点相关房间路径树能够插入新节点的位置

		//找到房间的入口节点
		PPathNode rootNode = BackwardToRoomRoot(currentNode);

		if (rootNode != NULL)
		{
			PPathNode posInsert = NULL;
			//初始化寻找可以插入的位置条件
			PPathNode brothers[1];
			brothers[0] = rootNode;

			posInsert = FindPositionInsertPathNode(brothers, 1, rootNode->room_->dimNum_);
			//printf("Node Position Insert,before\r\n");
			//DumpPath(posInsert);
			//printf("===========================\r\n");

			if (posInsert != NULL)
			{
				InsertPathNode(posInsert, _newNode, currentNode->path_);
				MoveRoomInPathRoomByRoom(&currentNode->path_->pathNodeRoom_, _newNode->room_);
				//printf("Node Position Insert,after\r\n");
				//DumpPath(posInsert);
				//printf("-------------------------\r\n");
			}
		}

		assert(_newNode->Parent_ != NULL);
		door->isClosed_ = 0;//将门标识为打开的
	}
	else
		assert(0);
}


//主要通路建立完成后整理已经建立的路径，将主路径置为最左路径
void ArrangeWhenMeetEndRoom(PPathNode _node)
{
	do
	{
		PPathNode parent = _node->Parent_;
		if (parent != NULL)
		{
			assert(parent->MainWay_ == _node || parent->SubWay_ == _node);
			if (parent->MainWay_ != _node)
			{
				parent->SubWay_ = parent->MainWay_;
				parent->MainWay_ = _node;
			}
		}
		_node = parent;
	} while (_node != NULL);
}

PPath BuildPath(PMaze _maze, PDim _beginDim, PDim _endDim)
{
	PPath ret;
	PPathNode currentNode = NULL;
	LinkLineControlP llc = OLL_Alloc();
	PreparePath(&ret, _maze, _beginDim->values_, _beginDim->length, _endDim->values_, _endDim->length);
	currentNode = ret->firstNode_;
	printf("Begin\r\n");

	//for (int i = 0; i < 10000; i++)
	//	printf("%03d ", Random(0, 100));
	do
	{
		PRoom room = currentNode->room_;

		PDoorSet doorsNotOpen = (PDoorSet)malloc(sizeof(DoorSet));
		InitializeDoorSet(doorsNotOpen);
		int allDoorsNotLinked = GetAllDoorsToRoomsNotInPath(room, doorsNotOpen, ret);
		if (allDoorsNotLinked > 0)
		{
			int singleRoomOpenedDoorCount = 0;
			for (int i = 0; i < allDoorsNotLinked; i++)
			{
				if (Random(1, 4) > 2)
				{
					PDoor door = doorsNotOpen->doors_[i];
					PRoom newLinkRoom = NULL;
					PPathNode newLinkNode = NULL;

					singleRoomOpenedDoorCount++;

					if (door->linkedRooms_[0] == room)
						newLinkRoom = door->linkedRooms_[1];
					else
						newLinkRoom = door->linkedRooms_[0];

					newLinkNode = AllocPathNode(newLinkRoom, ret);
					LinkCurrentNodeToNewRoomNode(&currentNode, newLinkNode);

					if (newLinkRoom == ret->roomEnd_)
					{
						ArrangeWhenMeetEndRoom(newLinkNode);
					}
					else
						OLL_Push(llc, newLinkNode);
					DumpPath(ret->firstNode_);
				}
			}
			printf("%d doors opened for one room.\r\n", singleRoomOpenedDoorCount);
		}
		currentNode = (PPathNode)OLL_Pop(llc);

		if (currentNode == NULL)
		{
			//找一个未连通且与已创建路径相邻的房间，打通这个房间和路径，重新开始
			PPRoom notInPathRooms = ret->pathNodeRoom_.roomOutPath_;
			int size = ret->pathNodeRoom_.outSize_;

			if (size > 0)
			{
				for (int i = 0; i < size; i++)
				{
					PRoom roomOutPath = notInPathRooms[i];
					PRoomWithNeighborInPath rwnpp = GetRoomNeighborInPath(roomOutPath, ret);
					if (rwnpp != NULL)
					{
						if (rwnpp->nearInPathRooms_ != NULL && rwnpp->nearRoomNumbers_ > 0)
						{
							int selectOpenRoomIndex = Random(0, rwnpp->nearRoomNumbers_);//选择一个房间打开
							PNearNeighborInPath nearRoomNNIP = &rwnpp->nearInPathRooms_[selectOpenRoomIndex];
							PPathNode node = nearRoomNNIP->pathNode_;
							PPathNode newNode = AllocPathNode(roomOutPath, ret);

							LinkCurrentNodeToNewRoomNode(&node, newNode);
							DumpPath(ret->firstNode_);
							OLL_Push(llc, newNode);
						}
						FreeRoomWithNeighborInPath(rwnpp);
					}
				}
			}
			currentNode = (PPathNode)OLL_Pop(llc);
		}
	} while (currentNode != NULL);
	OLL_Release(llc);

	DumpPath(ret->firstNode_);
	return ret;
}

PPathNode FindInPath(PPathNode _node, PRoom _room);
PPathNode FindByDim(PPath _path, PDim _dim)
{
	PRoom room = NULL;

	int size = _path->pathNodeRoom_.inSize_;
	for (int i = 0; i < size; i++)
	{
		PRoom room0 = _path->pathNodeRoom_.roomInPath_[i];
		if (room0 != NULL)
		{
			int j;
			assert(_dim->length == room0->dimNum_);
			for (j = 0; j < room0->dimNum_; j++)
				if (room0->dim_[j] != _dim->values_[j])
					break;

			if (j == room0->dimNum_)
			{
				room = room0;
				break;
			}
		}
	}

	if (room != NULL)
	{
		return FindInPath(_path->firstNode_, room);
	}
	return NULL;
}

PPathNode FindInPath(PPathNode _node, PRoom _room)
{
	PPathNode ret = NULL;
	if (_node->room_ == _room)
		return _node;

	if (_node->MainWay_ != NULL)
		ret = FindInPath(_node->MainWay_, _room);
	if (ret == NULL && _node->SubWay_ != NULL)
		ret = FindInPath(_node->SubWay_, _room);

	return ret;
}

PPath BuildPath0(PMaze _maze, PDim _beginDim, PDim _endDim)
{
	PPath ret;
	PPathNode currentNode = NULL;
	PreparePath(&ret, _maze, _beginDim->values_, _beginDim->length, _endDim->values_, _endDim->length);
	currentNode = ret->firstNode_;

	printf("Begin\r\n");
	//DumpRoom(currentNode->room_);

	do
	{
		if (currentNode != NULL)
		{
			PRoom room = currentNode->room_;

			printf("Current Room:");
			DumpRoom(currentNode->room_);
			if (room == ret->roomEnd_)//当当前房间是目的房间，整理整个树，使最左路径为主路径，并标识整个路径主路径已经确立，主路径上的节点为主路径节点
			{
				printf("Reach End Room\r\n");
				ArrangeWhenMeetEndRoom(currentNode);
				currentNode = BackwardPath(currentNode);

				if (currentNode == NULL)//如果回溯得到空值，则表示路径建立结束或者出错
				{

				}
				else
				{
					printf("Back to\r\n");
					DumpRoom(currentNode->room_);
				}
			}
			else
			{
				PDoorSet doorsNotOpen = (PDoorSet)malloc(sizeof(DoorSet));
				//DumpRoom(room);
				InitializeDoorSet(doorsNotOpen);

				int allDoorsNotLinked = GetAllDoorsToRoomsNotInPath(room, doorsNotOpen, ret);

				//DumpDoorSet(doorsNotOpen->doors_, doorsNotOpen->count_);
				//printf("$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");

				if (allDoorsNotLinked == 0)//如果没有能够分配的门，则回溯到还有路径未连接邻居的房间
				{
					currentNode = BackwardPath(currentNode);

					if (currentNode == NULL)//如果回溯得到空值，则表示路径建立结束或者出错
					{

					}
					else
					{
						printf("Back to\r\n");
						DumpRoom(currentNode->room_);
					}
				}
				else
				{
					//printf("Current Room:\r\n");
					//DumpRoom(currentNode->room_);
					//printf("Room End--------\r\n");

					PDoor door = RandomSelectDoor(doorsNotOpen);
					assert(door != NULL && (door->linkedRooms_[0] == room || door->linkedRooms_[1] == room));

					//DumpDoor(door, NULL);
					//printf("################\r\n");

					PRoom nextRoom = NULL;
					if (door->linkedRooms_[0] == room)
						nextRoom = door->linkedRooms_[1];
					else
						nextRoom = door->linkedRooms_[0];

					//创建一个链接房间的节点，并将其加入路径
					PPathNode newNode = AllocPathNode(nextRoom, ret);
					assert(newNode != NULL);

					//将两个节点中的房间通过门连接起来，并修改Path的两个数组
					LinkCurrentNodeToNewRoomNode(&currentNode, newNode);
					currentNode = newNode;

					//printf("Through\r\n");

				}
				FreeDoorSet(doorsNotOpen);
			}

		}
		else//从不在路径上的房间中随机选取一个房间，随机选择一个它的邻居打通，并将此房间作为当前节点 
			//暂时不写 目前的算法里不会导致这种情况出现
		{

		}
	} while (currentNode != NULL);
	//DumpPath(ret->firstNode_);
	//{
	//	Dim dim;
	//	dim.length = 3;
	//	dim.values_ = (int*)malloc(sizeof(int) * 3);
	//	dim.values_[0] = 2;
	//	dim.values_[1] = 2;
	//	dim.values_[2] = 2;
	//	PPathNode found = FindByRoom(ret, &dim);
	//	free(dim.values_);
	//}
	return ret;
}

//获取连接两个房间的门，不管门是否开闭
PDoor GetDoorBetweenRooms(PRoom _roomFrom, PRoom _roomTo)
{
	PDoor ret = NULL;
	int doorNum = _roomFrom->dimNum_ * 2;
	PPDoor door_p = NULL;

	//printf("\r\nRoom From:");
	//DumpRoom(_roomFrom);
	//printf("\r\nRoom To:");
	//DumpRoom(_roomTo);

	for (int i = 0; i < doorNum; i++)
	{
		door_p = (PPDoor)GetPointerPS(_roomFrom->Doors_->doors_, _roomFrom->Doors_->count_, i);
		if (door_p != NULL)
			if ((*door_p)->linkedRooms_[0] == _roomTo)
				return *door_p;
			else if ((*door_p)->linkedRooms_[1] == _roomTo)
				return *door_p;
	}
	return ret;
}

//查找一个房间所有的不在路径里的门
int GetAllDoorsToRoomsNotInPath(PRoom _room, PDoorSet _ret, PPath _path)
{
	assert(_path != NULL);
	assert(_room != NULL);

	DIMLENGTH dimNum = _path->maze_->dimNum_;
	int j = 0;
	InitializeDoorSet(_ret);
	AllocDoorSet(_ret, dimNum * 2);
	//获取所有的邻居房间
	PRoomSet rooms = GetNeighborRooms(_room, dimNum);

	//挑出不在路径上的房间
	for (int i = 0; i < rooms->count_; i++)
	{
		//两种方法判断门是不是在路径上
		//一种是只要门开着就表示门在路径上
		//一种是查找路径的缓存，这里用查找缓存的方式
		InPathOrNot ipn = RoomInPath(_path, rooms->rooms_[i]);

		if (ipn == ION_OUTPATH)
		{
			//找出连接这些邻居的门
			PDoor door = GetDoorBetweenRooms(_room, rooms->rooms_[i]);
			if (door != NULL && door->isClosed_)//门必须是已经关闭的
			{
				_ret->doors_[j++] = door;
			}
		}
	}

	_ret->count_ = j;

	FreeRoomSet(&rooms);
	return _ret->count_;
}

PRoomSet GetNeighborNotInPath(PRoom _room, PMaze _maze, PPath _path)
{
	PRoomSet ret = AllocRoomSet();
	PDoorSet doorset = GetRoomDoors(_room, _maze);
	PPRoom buffer;
	int size, i, count = 0;

	size = doorset->count_;
	buffer = (PPRoom)malloc(size * sizeof(PRoom));
	for (i = 0; i < size; i++)
	{
		PDoor door = doorset->doors_[i];
		PRoom room = NULL;
		if (door->linkedRooms_[0] == _room && door->linkedRooms_[1] != NULL)
		{
			room = door->linkedRooms_[1];
		}
		else if (door->linkedRooms_[1] == _room && door->linkedRooms_[0] != NULL)
		{
			room = door->linkedRooms_[0];
		}

		if (room != NULL)
		{
			if (RoomInPath(_path, room) == ION_OUTPATH)
			{
				buffer[count++] = room;
			}
		}
	}

	size = count * sizeof(PRoom);
	ret->count_ = count;
	ret->rooms_ = (PPRoom)malloc(size);
	memcpy(ret->rooms_, buffer, size);
	free(buffer);
	FreeDoorSet(doorset);
	return ret;
}

void DumpPath(PPathNode _path)
{
	static int tabNum = 0;
	for (int i = 0; i < tabNum; i++)
		printf(" ");
	if (_path == NULL)
	{
		printf("NULL Path\r\n");
		return;
	}

	printf
	("Parent:%s", (_path->Parent_ == NULL) ?
		"NULL Parent" :
		(
		(_path->Parent_->room_ == NULL) ?
			"NULL Parent Room" : DimValue2(_path->Parent_->room_->dim_, _path->Parent_->room_->dimNum_)
			)
	);
	printf
	("Room:%s\r\n",
		(_path->room_ == NULL) ?
		"NULL Parent Room" : DimValue2(_path->room_->dim_, _path->room_->dimNum_)
	);
	tabNum++;
	DumpPath(_path->MainWay_);
	DumpPath(_path->SubWay_);
	tabNum--;
}

PRoomWithNeighborInPath GetRoomNeighborInPath(PRoom _room, PPath _path)
{
	PRoomSet rooms = GetNeighborRooms(_room, 0);
	PRoomWithNeighborInPath rwnip = AllocRoomWithNeighborInPath(_room);
	for (int i = 0; i < rooms->count_; i++)
	{
		InPathOrNot ipon = RoomInPath(_path, rooms->rooms_[i]);
		if (ipon == ION_INPATH)
			AddNeighborToNNIP(rwnip, rooms->rooms_[i], _path);
	}

	FreeRoomSet(&rooms);
	return rwnip;
}

PRoomWithNeighborInPath AllocRoomWithNeighborInPath(PRoom _room)
{
	PRoomWithNeighborInPath ret = (PRoomWithNeighborInPath)malloc(sizeof(RoomWithNeighborInPath));
	ret->Room_ = _room;
	ret->nearRoomNumbers_ = 0;
	ret->nearInPathRooms_ = NULL;
	return ret;
}

void FreeRoomWithNeighborInPath(PRoomWithNeighborInPath _roomWNIP)
{
	if (_roomWNIP != NULL)
	{
		if (_roomWNIP->nearInPathRooms_ != NULL)
		{
			free(_roomWNIP->nearInPathRooms_);
		}
		free(_roomWNIP);
	}
}

void AddNeighborToNNIP(PRoomWithNeighborInPath _roomWNIP, PRoom _neighbor, PPath _path)
{
	if (_roomWNIP != NULL)
	{
		assert(_roomWNIP->nearInPathRooms_ != NULL || _roomWNIP->nearRoomNumbers_ == 0);
		PNearNeighborInPath buffer = _roomWNIP->nearInPathRooms_;

		_roomWNIP->nearInPathRooms_ = (PNearNeighborInPath)malloc(sizeof(NearNeighborInPath)*(_roomWNIP->nearRoomNumbers_ + 1));
		if (_roomWNIP->nearRoomNumbers_ > 0)
			memcpy(_roomWNIP->nearInPathRooms_, buffer, sizeof(NearNeighborInPath)*_roomWNIP->nearRoomNumbers_);

		PNearNeighborInPath repl = _roomWNIP->nearInPathRooms_ + _roomWNIP->nearRoomNumbers_;
		repl->linkDoor_ = GetDoorBetweenRooms(_neighbor, _roomWNIP->Room_);
		repl->pathNode_ = FindInPath(_path->firstNode_, _neighbor);
		assert(repl->pathNode_ != NULL);
		repl->room_ = _neighbor;

		_roomWNIP->nearRoomNumbers_++;

		if (buffer != NULL)
			free(buffer);
	}
}
