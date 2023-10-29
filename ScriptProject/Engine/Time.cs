using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class Time
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern float GetDeltaTime();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern float GetElapsedTime();
    }
}
