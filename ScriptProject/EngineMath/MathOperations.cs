using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.EngineMath
{
    internal class MathOperations
    {
        public static float Lerp(float start, float end, float t)
        {
            return start + (end - start) * t;
        }
    }
}
