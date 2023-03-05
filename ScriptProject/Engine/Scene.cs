using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class Scene
    {
        UInt32 m_scene_index;
        EntityManager m_entity_manager;

        public Scene(UInt32 scene_index)
        {
            m_scene_index= scene_index;
            m_entity_manager = new EntityManager(this);
        }

        public EntityManager GetEntityManager()
        {
            return m_entity_manager;
        }

        public UInt32 GetSceneIndex() 
        { 
            return m_scene_index;
        }
    }
}
