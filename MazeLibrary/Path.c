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

//��һ������Ӳ���·�����е�״̬�Ƶ�����·������״̬��
//_pr ·������
//_pos �÷�����·��������·���������е�λ��
void MoveRoomInPathRoom(PPathNodeRoom _pr, int _pos)
{
	int i;
	PPRoom buffer;
	int size;
	assert(_pos >= 0 && _pos < _pr->outSize_);

	PRoom room = _pr->roomOutPath_[_pos];
	//�Ƴ�����·����������
	size = (_pr->outSize_ - _pos - 1) * sizeof(PRoom);
	if (size > 0)
	{
		buffer = (PPRoom)malloc(size);
		memcpy(buffer, _pr->roomOutPath_ + _pos + 1, size);
		memcpy(_pr->roomOutPath_ + _pos, buffer, size);//�����ǰ��
		free(buffer);
	}

	_pr->outSize_--;

	PRoom room0 = NULL;
	//�ҵ�����·�����ϵĵ�һ���հ�λ�û��ߵ�һ����Ҫ�����ָ����λ��
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
		else//���룬�������еĺ���
		{
			assert(i <= _pr->inSize_ - 1);
			size = (_pr->inSize_ - i) * sizeof(PRoom);

			if (size > 0)
			{
				buffer = (PPRoom)malloc(size);//��Ҫһ������
				memcpy(buffer, _pr->roomInPath_ + i, size);
				memcpy(_pr->roomInPath_ + i + 1, buffer, size);
				free(buffer);
			}

			_pr->roomInPath_[i] = room;
			_pr->inSize_++;
		}
	}
	else if (_pr->inSize_ < _pr->maxSize_)//ֱ����β�����
	{
		_pr->roomInPath_[_pr->inSize_] = room;
		_pr->inSize_++;
	}

	DumpRoomSet(_pr->roomInPath_, _pr->inSize_);
	DumpRoomSet(_pr->roomOutPath_, _pr->outSize_);
}

//��ָ���ķ����·����ʼ������һ��·���ڵ�
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

//��ʼ��·��
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

//׼��·����������ʼ��·��������·������ڽڵ�
//_path �����´�����·����ڽڵ�
//_maze ��Ҫ����·�����Թ�����
//_dimEntry,_dimEntryLength ��ڵķ������꣬ȱʡʹ��(0,0,...)��Ϊ���
//_dimExit,_dimExitLength ���ڵķ������꣬ȱʡʹ������ά�����������Ϊ����
void PreparePath(PPPath _path, PMaze _maze, DIMCONTENT _dimEntry, DIMLENGTH _dimEntryLength, DIMCONTENT _dimExit, DIMLENGTH _dimExitLength)
{
	PRoom room = FindRoom(_dimEntryLength, _dimEntry, _maze);//�����ҵ���ڷ���
	int size;//·���������Ŀ
	int pos;//��ʼ������ڷ�����·�������ϵ�λ��

	if (room == NULL)//��ڷ���Ϊ�գ�ʹ��ȫ�����귿����Ϊ��ڷ���
	{
		DIMCONTENT dim = GenerateDefaultDim(_maze);
		room = FindRoom(_maze->dimNum_, dim, _maze);
		free(dim);
	}

	*_path = AllocPath();//����·������
	AllocPathNode(room, *_path);
	PPath path = *_path;

	path->firstNode_ = AllocPathNode(room, *_path);//ʹ����ڷ��䴴����ڽڵ�
	path->maze_ = _maze;//ָ��·��ʹ�õ��Թ�

	PPathNode pathNode = path->firstNode_;
	//��ʼ��·���ڵ��������
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

	//���з����ڳ�ʼ��ʱ�����벻��·���ϵķ���
	for (int i = 0; i < _maze->roomNum_; i++)
	{
		pathNodeRoom->roomOutPath_[i] =
			&_maze->rooms_[i];
		if (path->roomStart_ == &_maze->rooms_[i])
			pos = i;//��¼��ڷ����λ��
	}

	//���Ƚ���ڷ����Ƶ�����·���Ϸ�����
	MoveRoomInPathRoom(pathNodeRoom, pos);

	//���ҳ��ڷ���
	room = FindRoom(_dimExitLength, _dimExit, _maze);

	if (room == NULL)//������ڷ��䲻���ڣ�ʹ�ø�ά���ά�ȷ�����Ϊ���ڷ���
	{
		DIMCONTENT dim;
		dim = CopyDimEx(_maze->dims_, _maze->dimNum_);
		room = FindRoom(_maze->dimNum_, dim, _maze);
		free(dim);
	}

	path->roomEnd_ = room;//��ǳ��ڷ���
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

//�жϷ����Ƿ���·������
InPathOrNot RoomInPath(PPath _path, PRoom _room)
{
	PPathNodeRoom pr = &_path->pathNodeRoom_;
	InPathOrNot ret = ION_UNKNOWN;
	int i;

	//���¼�����������������ݣ������Ѿ����򣬿��Ըĳɶ��ַ�
	for (i = 0; i < pr->inSize_; i++)//��·���Ѱ������������в���ָ������
	{
		if (pr->roomInPath_[i] == _room)//�鵽��������·����
		{
			ret = ION_INPATH;
			break;
		}
		else if (pr->roomInPath_[i] > _room)//�鲻��
			break;
	}

	if (ret == ION_UNKNOWN)//�鲻������鲻��·���Ϸ�������
	{
		for (i = 0; i < pr->outSize_; i++)
		{
			if (pr->roomOutPath_[i] == _room)//�鵽�����䲻��·����
			{
				ret = ION_OUTPATH;
				break;
			}
			else if (pr->roomOutPath_[i] > _room)//�鲻��������
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

//�ڵ�ǰ·�����ڵķ������ѡ��һ������·�������ǿ����ھ���Ϊ��ǰ����
//							�����ǰ·������·��Ϊ�գ���Ϊ��·�������µ�·����
//								�������֧·��Ϊ�գ���Ϊ֧·�����뵱ǰ·����
//								���򣬽���ǰ·���ķ��俽������һ��·����Ϊ��ǰ·����
//										�丸·��ָ��֮ǰ�ĵ�ǰ·�����µ�·����Ϊ�µĵ�ǰ·������·������·����
//��������������ķ�������ݵ���һ�����䣨���ܾ������·���������ҽ����ݵ�·�����丸·������·��ȫ��������֧·
//����޷����ݣ���·��Ϊ�գ����Ӳ���·���ϵķ��������ѡ��һ��������Ϊ��ǰ���䣬
//							�ڸ÷�����ھ������ѡ��һ��·���ڷ��䣬����ͨ����֮��ķ��ţ�
//

//�ж�һ�����Ƿ������������ţ���������������һ������
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

//�����������������·��������������·����
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

//�жϰ���_path��·���Ƿ��Ѿ�ȷ����·��
static inline int isMainWayConfirmed(PPathNode _pathNode)
{
	assert(_pathNode != NULL);
	PPath path = _pathNode->path_;


	return (path->flag_.flags_.MainWayConfirmed_ != 0);
}

//�ҵ����ߴ���һ�����Ը�����·����·���ڵ�
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

//����ǰ·���ڵ�ָ���ķ���ͨ��ָ�����Ž���ĳ���ھ����ӵ�·����
PPathNode MakePath(PPPathNode _currentPath, PDoor _door, int _dimNum)
{
	PPathNode currentPath = *_currentPath;
	PRoom room = currentPath->room_;

	assert(room != NULL || room->Doors_ == NULL);
	int i;
	for (i = 0; i < _dimNum * 2; i++)//���ȼ��������ڴ˷���
	{
		PPDoor door_p = (PPDoor)GetPointerPS(room->Doors_->doors_, room->Doors_->count_, i);
		if (*door_p == _door)
			break;
	}

	if (i < _dimNum * 2)//�Ǵ˽ڵ��Ӧ�������
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
		//ToDo ��Ҫ�ѷ����·����δ�ڽڵ�������ת�Ƶ����ڽڵ�������
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

//����һ���ڵ㣬�����ʱ����·������·����δȷ�������ߵ�ǰ�ڵ�ĸ��ڵ㲻����·���ϵĽڵ㣬���򽻻����ڵ����֧�ڵ��λ�ã�
//							2018-4-24 ��������֧�ڵ㲢���Ͻ�������Ҫ����ǰ�ڵ��Ƶ���󣬿ճ����ڵ�λ�ã�
//							2018-4-26 ��ȫ��������·��ȷ���Ժ��ع���·���ṹ���Դ˼�·��������Ҳ���õ����ƻ����������ƽ��������Ľṹ
//										�����ڻ���ʱ�Ͳ��ÿ������ӷ�·������
//						���޸���������Ϊ ����ýڵ��ڸ��ڵ��еķ�·λ���Ƿ�·�ڵ㣬���ҵ��÷���Ĳ���λ�ã������˽ڵ���뵽�ò���λ���ϣ�ԭ����·��NULL)
//����ֵΪ�������ﵽ�Ľڵ㣬������������ݵĽڵ㷿�䲻ͬ�����ú�������ѡ�񷿼�
//���͵Ļ��ݲ����ǵ�ǰ�ڵ�ķ����Ѿ��޷������µ�·���������Ҫ���ش˷������ڷ��������ͼ����·��
//ע�������ݣ��ҵ���·���ϵ�һ���в���·���ϵ����ڷ��伴�ɣ�����NULL��ʾ·�����ɽ���
PPathNode BackwardPath(PPathNode _currentPath)
{
	PPathNode RoomEntryNode = BackwardToRoomRoot(_currentPath);//�����ҵ���ǰ�������ڽڵ㣨���ýڵ�ĸ��ڵ㲢�ǵ�ǰ���䣩
	if (RoomEntryNode != NULL)//������ڵ����
	{
		//��ȡ��ڽڵ�
		PPathNode parentNode = RoomEntryNode->Parent_;

		if (parentNode != NULL)
		{
			//��ȡ��ڷ���
			PRoom room = parentNode->room_;

			//��ڷ�������в���Path�ϵ��ھӷ��䣬�򷵻ظýڵ㣬������ֹ������ݹ���ݣ�ֱ���޷����ݣ����ؿ�ֵ
			//printf("Back throw room:");
			//DumpRoom(room);

			//��鵱ǰ������ھӣ��Ƿ��в���Path�ϵ����
			PRoomSet neighbors = GetNeighborRooms(room, DimNumberOfPathNode(_currentPath));//��ȡ��ǰ������ھ�

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
						return parentNode;//��ǰ�����д����ھӲ���·���ϣ����ص�ǰ����
					}
				}

				FreeRoomSet(&neighbors);
				//�ݹ���ҵ�ǰ�������һ��
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
	//����Ҳ�����ڽڵ㣬˵���޷�����
	return NULL;
}

//�ڽڵ����/�η�·�ϲ���һ��ָ������Ľڵ㣬���ز���Ľڵ�
//�����ǰ�ڵ������·��Ϊ�����½ڵ㸳���η�·�����򸳸�����·
//10-18 �����⣬���η�·���ǿ�ʱ��Ȼ���Բ��룬��Ѱ�ҿɲ���ڵ�ʱ���뷵�ز���λ��������·���Ǵη�·����һ������������Ҫ����ѡ��·ָ�򷿼�͵�ǰ���䲻һ�µģ����������·����һ�����������·����Ȼ�������ԭ��·ָ��Ľڵ�����ǿձ����޸���ڽڵ�
//_currentNode �Ѿ��ж����Բ����½ڵ�Ľڵ�
//_room �²���ڵ�ķ���
PPathNode InsertPathNode(PPathNode _currentNode, PPathNode _nodeInsert, PPath _path)
{
	assert(_currentNode != NULL);

	//��ӽڵ�Ľڵ�λ�� ����ӵ�·���еĽڵ�
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
	_nodeInsert->MainWay_ = *nodeTobeInsert;//�ɵ��ӽڵ�ҵ�����Ľڵ������·�� ��ʹԭֵΪ��
	if (nodeTobeInsert != NULL && *nodeTobeInsert != NULL)
	{
		(*nodeTobeInsert)->Parent_ = _nodeInsert;
		//printf("Node To Be Insert:");
		//DumpPath(*nodeTobeInsert);
		//printf("&&&&&&&&&&&&&&&&&&&&&&\r\n");
	}

	*nodeTobeInsert = _nodeInsert;//����Ľڵ�ҵ�����Ľڵ�λ��

	_nodeInsert->Parent_ = _currentNode;
	//printf("Node Inserted:");
	//DumpPath(_nodeInsert);
	//printf("&&&&&&&&&&&&&&&&&&&&&&\r\n");

	return _nodeInsert;//���ر�����Ľڵ�
}

//����֪������ͬ��·���ڵ����ҵ���һ���ܹ�����µĴӸ÷��������·���ڵ��ָ��λ��
//_nodes ������ͬһ������ص�·���ڵ���ͬһ����ϵ��ֵܽڵ�
//_nodesNum ��_nodes��Ľڵ���Ŀ
//_dimNum ά�ȵ���Ŀ
//_isSubWay ����ӵ�λ���Ƿ��Ƿ��ؽڵ��֧·�ڵ� //10-2 ȥ��? ȥ����
//����ֵ �ҵ�������ӻ��߲���Ľڵ�ĸ��ڵ�
//�˺����ǵݹ�ģ�����ڵ�ǰ�ڵ��Ҳ���λ�ã����ռ�������ͬ��ε�һ���ӽڵ�ݹ����
PPathNode FindPositionInsertPathNode(PPPathNode _nodes, int _nodesNum, int _dimNum)
{
	int i, size;
	PPPathNode subNodes = NULL;//���������һ��Ľڵ�
	PPPathNode currentSubNodesP = NULL;//���ڱ����ĵ�ǰ��һ��ڵ�洢��λ��
	PPathNode ret = NULL;
	PRoom room = NULL;
	PPathNode node0 = _nodes[0];//��Ҫ�м���븴�ƽڵ��λ��

	size = _dimNum * 2;
	subNodes = (PPPathNode)malloc(sizeof(PPathNode)*size);//Ԥ���������ܿռ䣬ÿ����һ��
	memset(subNodes, 0, sizeof(PPathNode)*size);//��ʼ����һ��ڵ�����ָ��Ϊ��

	currentSubNodesP = subNodes;
	//���ڵ���������˵�������ж��·���ڵ㣬����Щ�ڵ���ƽ��������Ľṹ�洢
	//�������д洢��_nodes��Ľڵ�,������ӽڵ�
	for (i = 0; i < _nodesNum; i++)
	{
		PPathNode node = _nodes[i];
		if (node != NULL)
		{
			if (room == NULL)
				room = node->room_;

			PPathNode nodeSubMain = node->MainWay_;
			PPathNode nodeSubSub = node->SubWay_;

			if (nodeSubMain == NULL)//�յ�λ��->��� 
			{
				ret = node;
				break;
			}
			else if (nodeSubMain->room_ != room && nodeSubSub != NULL)//�²�����������->����
			{
				//	PPathNode newCloneNode;//�����ڵ㿽��һ����Ϊ����ڵ㣬�����뵽
				//	newCloneNode = AllocPathNode(room, node->path_);

				//	newCloneNode->enterDoor_ = node->enterDoor_;
				//	newCloneNode->Parent_ = node;
				//	newCloneNode->MainWay_ = node->MainWay_;
				//	node->MainWay_ = newCloneNode;
				//	ret = newCloneNode;
				node0 = node;
				break;
			}
			else if (nodeSubMain->room_ == node->room_)//��¼���ӽڵ㵽subNodes
			{
				if (nodeSubMain)
					*currentSubNodesP++ = node->MainWay_;
			}


			if (nodeSubSub == NULL)  //�յ�λ��->��� 
			{
				ret = node;
				break;
			}
			else if (nodeSubSub->room_ != room)//�²�����������->����
			{
				//	PPathNode newCloneNode;//�����ڵ㿽��һ����Ϊ����ڵ㣬�����뵽
				//	newCloneNode = AllocPathNode(room, node->path_);

				//	newCloneNode->enterDoor_ = node->enterDoor_;
				//	newCloneNode->Parent_ = node;
				//	newCloneNode->MainWay_ = node->MainWay_;
				//	node->SubWay_ = newCloneNode;
				//	ret = newCloneNode;
				node0 = node;
				break;
			}
			else if (nodeSubSub->room_ == node->room_)//��¼���ӽڵ㵽subNodes
			{
				if (nodeSubSub)
					*currentSubNodesP++ = node->SubWay_;
			}
		}
	}

	if (ret == NULL)
	{
		//���е�������ζ�ŵ�ǰ���Ҳ�����������ӽڵ��λ�ã��ݹ����
			//Ӧ�ò������������� 11-30 ��һ������򿪵��ų��������ͻ�����������
		if (currentSubNodesP == subNodes)//һ��������ϲ�����ȫ��������û���ӽڵ��ṩ�ݹ����
		{
			assert(node0 != NULL);

			//�´���һ��ͬһ����Ľڵ���뵽��һ���ӽڵ����棬����ԭ���Ľڵ�ҵ�����ڵ�ķ�·�ڵ��ϣ�����������ڵ㣬��������·�ڵ���Ϊ�����
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

//�����ҵ���ָ���ڵ�ͬһ����ĸ��ڵ�
PPathNode BackwardToRoomRoot(PPathNode _node)
{
	while (_node->Parent_ != NULL && _node->Parent_->room_ == _node->room_)
	{
		//if (_node->Parent_ != NULL)//��ǰ����ĸ��ڵ㲻�����������ĸ��ڵ�
		_node = _node->Parent_;
	}
	return _node;
}

PDoor RandomSelectDoor(PDoorSet _doors)
{
	return _doors->doors_[Random(0, _doors->count_)];
}

//�������ڵ��еķ���ͨ�����������������޸�Path���������飬���ŵ�·�����ϣ��ڵ�ǰ�ڵ���Ѱ���ܹ������λ�ã�
void LinkCurrentNodeToNewRoomNode(PPPathNode _currentNode, PPathNode _newNode)
{
	assert(_currentNode != NULL);
	assert(*_currentNode != NULL);

	PPathNode currentNode = *_currentNode;
	//���ҳ����������������
	PDoor door = GetDoorBetweenRooms(currentNode->room_, _newNode->room_);
	if (door != NULL)
	{
		//�ҳ���ǰ�ڵ���ط���·�����ܹ������½ڵ��λ��

		//�ҵ��������ڽڵ�
		PPathNode rootNode = BackwardToRoomRoot(currentNode);

		if (rootNode != NULL)
		{
			PPathNode posInsert = NULL;
			//��ʼ��Ѱ�ҿ��Բ����λ������
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
		door->isClosed_ = 0;//���ű�ʶΪ�򿪵�
	}
	else
		assert(0);
}


//��Ҫͨ·������ɺ������Ѿ�������·��������·����Ϊ����·��
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
			//��һ��δ��ͨ�����Ѵ���·�����ڵķ��䣬��ͨ��������·�������¿�ʼ
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
							int selectOpenRoomIndex = Random(0, rwnpp->nearRoomNumbers_);//ѡ��һ�������
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
			if (room == ret->roomEnd_)//����ǰ������Ŀ�ķ��䣬������������ʹ����·��Ϊ��·��������ʶ����·����·���Ѿ�ȷ������·���ϵĽڵ�Ϊ��·���ڵ�
			{
				printf("Reach End Room\r\n");
				ArrangeWhenMeetEndRoom(currentNode);
				currentNode = BackwardPath(currentNode);

				if (currentNode == NULL)//������ݵõ���ֵ�����ʾ·�������������߳���
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

				if (allDoorsNotLinked == 0)//���û���ܹ�������ţ�����ݵ�����·��δ�����ھӵķ���
				{
					currentNode = BackwardPath(currentNode);

					if (currentNode == NULL)//������ݵõ���ֵ�����ʾ·�������������߳���
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

					//����һ�����ӷ���Ľڵ㣬���������·��
					PPathNode newNode = AllocPathNode(nextRoom, ret);
					assert(newNode != NULL);

					//�������ڵ��еķ���ͨ�����������������޸�Path����������
					LinkCurrentNodeToNewRoomNode(&currentNode, newNode);
					currentNode = newNode;

					//printf("Through\r\n");

				}
				FreeDoorSet(doorsNotOpen);
			}

		}
		else//�Ӳ���·���ϵķ��������ѡȡһ�����䣬���ѡ��һ�������ھӴ�ͨ�������˷�����Ϊ��ǰ�ڵ� 
			//��ʱ��д Ŀǰ���㷨�ﲻ�ᵼ�������������
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

//��ȡ��������������ţ��������Ƿ񿪱�
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

//����һ���������еĲ���·�������
int GetAllDoorsToRoomsNotInPath(PRoom _room, PDoorSet _ret, PPath _path)
{
	assert(_path != NULL);
	assert(_room != NULL);

	DIMLENGTH dimNum = _path->maze_->dimNum_;
	int j = 0;
	InitializeDoorSet(_ret);
	AllocDoorSet(_ret, dimNum * 2);
	//��ȡ���е��ھӷ���
	PRoomSet rooms = GetNeighborRooms(_room, dimNum);

	//��������·���ϵķ���
	for (int i = 0; i < rooms->count_; i++)
	{
		//���ַ����ж����ǲ�����·����
		//һ����ֻҪ�ſ��žͱ�ʾ����·����
		//һ���ǲ���·���Ļ��棬�����ò��һ���ķ�ʽ
		InPathOrNot ipn = RoomInPath(_path, rooms->rooms_[i]);

		if (ipn == ION_OUTPATH)
		{
			//�ҳ�������Щ�ھӵ���
			PDoor door = GetDoorBetweenRooms(_room, rooms->rooms_[i]);
			if (door != NULL && door->isClosed_)//�ű������Ѿ��رյ�
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
