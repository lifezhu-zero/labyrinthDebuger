using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace labyrinthDebuger
{
    //维度坐标向量
    class Dimemsion:ICloneable
    {
        public Dimemsion(MazeSpace _s,int[] _v)
        {
            _v = _s.RegularDimemsion(_v);
            v_=(int[])_v.Clone();
        }

        object ICloneable.Clone()
        {
            return null;
        }

        private int[] v_;

        public int[] Value
        {
            get
            {
                return (int[])v_.Clone();
            }
        }
    }
}
