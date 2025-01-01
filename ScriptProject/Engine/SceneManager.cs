using System;
using System.Runtime.CompilerServices;

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

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern void RestartActiveScene();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern Scene GetGlobalScene();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern Scene LoadScene(string scene_name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern Scene LoadSceneSynchronized(string scene_name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static private extern bool IsSceneLoaded_External(UInt32 scene_index);

        static public bool IsSceneLoaded(Scene scene)
        {
            return IsSceneLoaded_External(scene.GetSceneIndex());
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static private extern void ChangeScene_External(UInt32 scene_index);

        static public void ChangeScene(Scene scene)
        {
            ChangeScene_External(scene.GetSceneIndex());
        }
    }
}
