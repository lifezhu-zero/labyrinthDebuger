using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace labyrinthDebuger
{
    //绘制或者行走时经历的路径，包括支路和主路，路径的主体是Walk
    class Path
    {
        public Room GetParent(Room _room)
        {
            return Room.NullRoom;
        }

        public void LinkRoom(Room _from,Room _to)
        {
            
        }

        public bool RoomInPath(Room _rm)
        {
            return false;
        }
    }
}
