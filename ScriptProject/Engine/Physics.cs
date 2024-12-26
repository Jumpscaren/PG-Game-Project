using ScriptProject.EngineMath;
using System;
using System.Runtime.CompilerServices;

namespace ScriptProject.Engine
{
    internal class Physics
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern RaycastResult Raycast(Vector2 position, Vector2 direction, UInt16 category, UInt16 mask, Int16 group);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static private extern bool RaycastCheckIfClosest_Extern(Vector2 position, Vector2 direction, UInt16 category, UInt16 mask, Int16 group, UInt32 scene_index, UInt32 entity);

        static public bool RaycastCheckIfClosest(Vector2 position, Vector2 direction, UInt16 category, UInt16 mask, Int16 group, GameObject checkIfClosest)
        {
            return RaycastCheckIfClosest_Extern(position, direction, category, mask, group, checkIfClosest.GetSceneIndex(), checkIfClosest.GetEntityID());
        }
    }
}
