using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace labyrinthDebuger
{
    //迷宫生成类
    class MazeCreator
    {
        public static Maze CreateMaze(int[] _vector)
        {
            //首先根据数组整理出维度，作为新建迷宫空间的各维尺度，即最大维度
            Dimemsion dim = new Dimemsion(null,_vector);

            //生成空间
            MazeSpace space = new MazeSpace(dim);

            //初始化全封闭的初始迷宫
            Maze ret = new Maze(space);

            //生成全连通图,并创建生成路径

            Path path = new Path();

            //生成迷宫的算法，暂时写在这里，以后算法会独立出来
            Room currentRoom = Room.NullRoom;

            do
            {
                if(!Room.ValidateRoom(currentRoom))
                    currentRoom = ret.RandomSelectRoomNotInPath(path);

                if (Room.ValidateRoom(currentRoom))
                {
                    MazeWall[] walls=currentRoom.Walls;
                    if(walls.Length>0)//当前房间还有没有打通的墙壁
                    {
                        //筛选出对面的房间不在路径上的墙壁
                        List<MazeWall> walls2 = new List<MazeWall>();
                        foreach (MazeWall w in walls)
                        {
                            if(!path.RoomInPath(w.NextRoom(currentRoom)))
                            {
                                walls2.Add(w);
                            }
                        }

                        if(walls2.Count()>0)
                        {
                            //随机选取一个墙壁打通
                            MazeWall wallForOpen = walls2[RandFunc.Rnd(0, walls2.Count() - 1)];
                            wallForOpen.MakeOpen();
                            Room nextRoom = wallForOpen.NextRoom(currentRoom);
                            path.LinkRoom(currentRoom, nextRoom);
                            currentRoom = nextRoom;
                        }
                        else if(currentRoom.Sealed)//一种情况是孤立的房间，没有和其它任何区域相通，而且周围都是开放的区域
                        {
                            //这种情况下，随机选取一个墙壁打通，并将对面的房间在路径中标记为自己的父节点，然后置当前房间为空
                            MazeWall wallForOpen = walls[RandFunc.Rnd(0, walls.Count() - 1)];
                            wallForOpen.MakeOpen();
                            Room prevRoom = wallForOpen.NextRoom(currentRoom);
                            path.LinkRoom(prevRoom, currentRoom);
                            currentRoom = Room.NullRoom;
                        }
                        else//退回到上一个房间
                        {
                            currentRoom = path.GetParent(currentRoom);
                        }
                    }
                    else
                    {
                        //退回到上一步
                        currentRoom = path.GetParent(currentRoom);
                    }
                }
            } while (!ret.AllRoomInPath(path));
            return Maze.NullMaze;
        }

        
    }
}
