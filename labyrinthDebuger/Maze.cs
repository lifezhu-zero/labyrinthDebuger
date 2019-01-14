using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace labyrinthDebuger
{
    //由房间和墙组合而成的迷宫整体
    class Maze
    {
        public Maze(MazeSpace _space)//初始化迷宫，全封闭，充满空间
        {
            
        }

        //随机在迷宫里寻找一个不在路径上的房间
        public Room RandomSelectRoomNotInPath(Path _path)
        {
            return Room.NullRoom;
        }

        /// <summary>
        /// 是否所有房间都已经归入路径
        /// </summary>
        /// <param name="_path"></param>
        /// <returns></returns>
        public Boolean AllRoomInPath(Path _path)
        {
            return false;
        }

        public static Maze NullMaze;
        protected List<List<Room>> rooms_;//各个维度的房间集合

        static Maze()
        {
            NullMaze=null;
        }

    }
}
