using ScriptProject.Engine;
using System;
using System.Collections.Generic;

namespace ScriptProject.Scripts
{
    internal class GameMaster : ScriptingBehaviour
    {
        List<Scene> scenes = new List<Scene>();
        public static GameMaster game_master = null;
        Scene changing_scene = null;

        void Start()
        {
            if (game_master != null)
            {
                Console.WriteLine("ERROR GAME MASTER ALREADY EXISTS");
                return;
            }

            //scenes.Add(SceneManager.LoadSceneSynchronized("temp"));
            scenes.Add(SceneManager.LoadSceneSynchronized("lvl_1"));
            scenes.Add(SceneManager.LoadScene("temp"));
            //scenes.Add(SceneManager.LoadScene("test_area_1"));
            scenes.Add(SceneManager.LoadScene("man"));

            SceneManager.ChangeScene(scenes[0]);

            scenes.RemoveAt(0);

            game_master = this;
        }

        public void NextScene()
        {
            if (changing_scene != null)
            {
                if (SceneManager.GetActiveScene() == changing_scene)
                {
                    changing_scene = null;
                }
                else
                {
                    Console.WriteLine("Skip beacuse scene is changing");
                }
                return;
            }

            if (scenes.Count == 0)
            {
                SceneManager.RestartActiveScene();
                return;
            }

            SceneManager.ChangeScene(scenes[0]);
            changing_scene = scenes[0];
            scenes.RemoveAt(0);
        }
    }
}
