using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class SceneManager
    {
        static Scene m_active_scene = null;

        static public Scene GetActiveScene()
        {
            if (m_active_scene == null || m_active_scene.GetSceneIndex() != GetActiveSceneIndex())
                m_active_scene = new Scene(GetActiveSceneIndex());

            return m_active_scene;
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static private extern UInt32 GetActiveSceneIndex();
    }
}
