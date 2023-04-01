using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class Transform : Component
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static private extern void SetPosition_Extern(UInt32 scene_index, UInt32 entity, float x, float y, float z);

        public void SetPosition(float x, float y, float z)
        {
            SetPosition_Extern(game_object.GetSceneIndex(), game_object.GetEntityID(), x, y, z);
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern void InitComponent(UInt32 scene_index, UInt32 entity);
    }
}
