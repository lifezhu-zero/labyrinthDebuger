using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace labyrinthDebuger
{
    //隔开各个房间的墙，墙是一种Division，墙只能是封闭的
    //打开的墙会被Door取代，所有变成门的墙在所有引用中都会被Door取代，因此墙不能直接通过Wall引用，必须通过Division引用
    //从一个房间对一堵开放的墙进行操作，将进入相邻的房间，每个墙连接且仅连接两个房间
    //主体包括两侧的房间 
    class MazeWall
    {
        protected MazeWall()
        {

        }

        public void MakeOpen()
        {

        }

        public Room NextRoom(Room _from)
        {
            return Room.NullRoom;
        }
    }
}
