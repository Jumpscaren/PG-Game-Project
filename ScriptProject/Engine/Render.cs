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
        static private extern Texture LoadTexture_External(string texture_name, UInt32 scene_index);
        static public void LoadTexture(string texture_name, Sprite sprite)
        {
            sprite.SetTexture(LoadTexture_External(texture_name, sprite.GetGameOjbect().GetSceneIndex()));
        }
    }
}
