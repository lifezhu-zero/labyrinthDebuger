using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace labyrinthDebuger
{
    //迷宫中基本的房间单元，房间在各个方向上被墙围绕，并且在迷宫中占据指定坐标的位置，所有房间的大小都是一样的
    class Room
    {
        public static Room NullRoom=new Room();

        public bool Validate
        {
            get { return this.Equals(NullRoom); }
        }

        public static bool ValidateRoom(Room _r)
        {
            return _r != null && _r.Validate;
        }

        public Room[] Neighbors
        {
            get { return new Room[0]; }
        }

        public Room[] NeighborsNotLinked
        {
            get { return new Room[0]; }
        }

        public MazeWall[] Walls
        {
            get { return new MazeWall[0]; }
        }

        //Room是否全封闭
        public Boolean Sealed
        {
            get { return true; }
        }

        private List<MazeWall> Walls_;
    }
}
