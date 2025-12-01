using ScriptProject.Engine;
using ScriptProject.EngineMath;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ScriptProject.UserDefined;

namespace ScriptProject.Scripts
{
    internal class DoorSpawner : ScriptingBehaviour
    {
        public string spawn_prefab_name = "";
        public float spawn_time = 0.0f;
        public float delay_initial_spawn_time = 0.0f;

        float spawn_timer = 0.0f;

        List<GameObject> spawn_points = new List<GameObject>();
        RandomGenerator random_generator = new RandomGenerator();

        void Start()
        {
            spawn_timer = delay_initial_spawn_time;

            spawn_points.Add(GameObject.FindGameObjectWithTag(UserTags.StartDoor));
            spawn_points.Add(GameObject.FindGameObjectWithTag(UserTags.Finish));
        }

        void Update()
        {
            if (spawn_timer < Time.GetElapsedTime())
            {
                int spawn_point_index = random_generator.RandomInt(0, spawn_points.Count - 1);
                GameObject spawn_point = spawn_points[spawn_point_index];

                GameObject new_game_object = GameObject.CreateGameObject();
                new_game_object.AddComponent<Sprite>();
                new_game_object.transform.SetPosition(spawn_point.transform.GetPosition());
                new_game_object.transform.SetZIndex(1);
                PrefabSystem.InstanceUserPrefab(new_game_object, spawn_prefab_name);

                spawn_timer = Time.GetElapsedTime() + spawn_time;
            }
        }
    }
}
