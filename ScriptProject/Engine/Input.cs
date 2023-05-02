using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.CompilerServices;

namespace ScriptProject.Engine
{
    internal class Input
    {
        //When key is pressed
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern bool GetKeyPressed();

        //When key is down
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern bool GetKeyDown();
    }
}
