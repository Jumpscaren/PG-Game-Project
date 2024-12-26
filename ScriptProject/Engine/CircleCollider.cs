using System;
using System.Runtime.CompilerServices;

namespace ScriptProject.Engine
{
    internal class CircleCollider : Component
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern void InitComponent(UInt32 scene_index, UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern bool HasComponent(UInt32 scene_index, UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern void RemoveComponent(UInt32 scene_index, UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetTrigger(bool trigger);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetColliderFilter(UInt16 category, UInt16 mask, Int16 group);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetRadius(float radius);
    }
}
