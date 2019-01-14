using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace labyrinthDebuger
{
    static class RandFunc
    {
        static RandFunc()
        {
            DateTime tm = DateTime.Now;
            RND = new Random((int)tm.Ticks);
        }

        static public int Rnd(int _min,int _max)
        {
            return RND.Next(_min, _max);
        }

        static private Random RND;
    }
}
