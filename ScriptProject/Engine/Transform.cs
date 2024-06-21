using ScriptProject.EngineMath;
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
        Cache<Vector2> cached_position = new Cache<Vector2>();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static private extern void SetPosition_Extern(UInt32 scene_index, UInt32 entity, float x, float y);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static private extern void SetLocalPosition_Extern(UInt32 scene_index, UInt32 entity, float x, float y);

        public void SetPosition(Vector2 position)
        {
            cached_position.CacheData(position);
            SetPosition_Extern(game_object.GetSceneIndex(), game_object.GetEntityID(), position.x, position.y);
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static private extern Vector2 GetPosition_Extern(UInt32 scene_index, UInt32 entity);

        public Vector2 GetPosition()
        { 
            if (cached_position.IsDataOld())
            {
                cached_position.CacheData(GetPosition_Extern(game_object.GetSceneIndex(), game_object.GetEntityID()));
            }

            return cached_position.GetData();
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern Vector2 GetLocalPosition();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetLocalRotation(float angle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern float GetLocalRotation();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern void InitComponent(UInt32 scene_index, UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern bool HasComponent(uint scene_index, uint entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern void RemoveComponent(uint scene_index, uint entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetZIndex(float z_index);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern float GetZIndex();
    }
}
