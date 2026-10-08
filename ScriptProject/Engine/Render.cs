using ScriptProject.Engine.Types;
using ScriptProject.EngineMath;
using System;
using System.Runtime.CompilerServices;

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

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern Vector2 GetFixedRenderSize();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern Vector2 GetWindowSize();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern float GetPixelsPerUnit();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static private extern UInt32 GetMaterial_External(string material_parameter_name);

        static public Material GetMaterial(string material_name) { return new Material(GetMaterial_External(material_name)); }
    }
}
