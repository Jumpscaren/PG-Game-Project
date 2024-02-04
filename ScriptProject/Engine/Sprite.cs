using ScriptProject.EngineMath;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class Sprite : Component
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern void InitComponent(UInt32 scene_index, UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern bool HasComponent(uint scene_index, uint entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern void RemoveComponent(uint scene_index, uint entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetTexture(Texture texture);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void FlipX(bool flip_x);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void FlipY(bool flip_y);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern bool GetFlipX();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern bool GetFlipY();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetUV(Vector2 uv_1_position, Vector2 uv_4_position);
    }
}
