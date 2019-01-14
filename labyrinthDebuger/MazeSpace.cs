using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace labyrinthDebuger
{
    //迷宫所在的空间的维度及各维尺度坐标
    class MazeSpace
    {
        public MazeSpace(Dimemsion _dim)
        {

        }

        //维度的空间限制，各维的最大尺度，从1开始计算
        public Dimemsion DIM
        {
            get
            {
                return dim_;
            }
        }

        private Dimemsion dim_;

        public int[] RegularDimemsion(int[] _i)
        {
            List<int> ret = new List<int>(DIM.Value);

            for (int i = 0; i < ret.Count(); i++)
            {
                if (i >= _i.Length)
                    ret[i] = 0;
                else if (_i[i] < 0)
                    ret[i] = 0;
            }

            return ret.ToArray();
        }
    }
}

