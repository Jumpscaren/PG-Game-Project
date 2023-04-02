using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class Render
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern Texture LoadTexture(string texture_name);
        //static public extern Texture LoadTexture(int g);
    }
}
