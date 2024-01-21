using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class AnimationManager
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern void SaveAnimation(GameObject game_object, string animation_file_name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern void LoadAnimation(GameObject game_object, string animation_file_name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern bool IsAnimationPlaying(GameObject game_object, string animation_file_name);
    }
}
