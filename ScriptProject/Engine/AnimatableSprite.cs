using ScriptProject.EngineMath;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class AnimatableSprite : Component
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern void InitComponent(UInt32 scene_index, UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern bool HasComponent(uint scene_index, uint entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern void RemoveComponent(uint scene_index, uint entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetLoop(bool loop);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetId(UInt32 id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetAnimationSpeed(float speed);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void ResetAnimation();
    }
}
