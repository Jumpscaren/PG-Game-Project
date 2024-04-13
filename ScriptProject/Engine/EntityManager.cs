using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class EntityManager
    {
        Scene m_scene;

        public EntityManager(Scene scene) 
        {
            m_scene = scene;
        }

        public UInt32 NewEntity()
        {
            return CreateEntity(m_scene.GetSceneIndex());
        }

        public void RemoveEntity(GameObject gameObject)
        {
            DeleteEntity(m_scene.GetSceneIndex(), gameObject.GetEntityID());
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static private extern UInt32 CreateEntity(UInt32 scene_index);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static private extern void DeleteEntity(UInt32 scene_index, UInt32 entity);
    }
}
