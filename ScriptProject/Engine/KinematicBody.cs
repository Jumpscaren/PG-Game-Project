using ScriptProject.EngineMath;
using System;
using System.Runtime.CompilerServices;

namespace ScriptProject.Engine
{
    internal class KinematicBody : Component
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern void InitComponent(UInt32 scene_index, UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern bool HasComponent(UInt32 scene_index, UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern void RemoveComponent(UInt32 scene_index, UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetVelocity(Vector2 velocity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern Vector2 GetVelocity();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetFixedRotation(bool fixed_rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetEnabled(bool enabled);
    }
}
