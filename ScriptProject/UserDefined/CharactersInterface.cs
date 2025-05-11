using ScriptProject.Engine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.UserDefined
{
    class CharactersInterface
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern void GetInteractiveCharacters(ListSetGameObject list);
    }
}
