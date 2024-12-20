﻿using ScriptProject.EngineMath;
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
        private Cache<Vector2> cache_velocity = new Cache<Vector2>();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern void InitComponent(UInt32 scene_index, UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern bool HasComponent(UInt32 scene_index, UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern void RemoveComponent(UInt32 scene_index, UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern Vector2 SetVelocity_Extern(UInt32 scene_index, UInt32 entity, Vector2 velocity);

        public void SetVelocity(Vector2 velocity)
        {
            cache_velocity.CacheData(velocity);
            SetVelocity_Extern(game_object.GetSceneIndex(), game_object.GetEntityID(), velocity);
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern Vector2 GetVelocity_Extern(UInt32 scene_index, UInt32 entity);

        public Vector2 GetVelocity()
        {
            if (cache_velocity.IsDataOld())
            {
                cache_velocity.CacheData(GetVelocity_Extern(game_object.GetSceneIndex(), game_object.GetEntityID()));
            }

            return cache_velocity.GetData();
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetFixedRotation(bool fixed_rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetEnabled(bool enabled);
    }
}
