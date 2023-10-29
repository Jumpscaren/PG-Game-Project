﻿using ScriptProject.Math;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class DynamicBody : Component
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
    }
}
