using ScriptProject.Engine;
using ScriptProject.EngineMath;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Scripts
{
    internal class Spawner : InteractiveCharacterBehaviour
    {
        public string spawn_prefab_name = "";
        public float spawn_time = 0.0f;
        public float delay_initial_spawn_time = 0.0f;

        float spawn_timer = 0.0f;

        RandomGenerator random_generator = new RandomGenerator();

        float health = 100.0f;

        enum SpawnerState
        {
            Idle,
            Spawning
        }

        public bool spawn_left = false;
        public bool spawn_right = false;
        public bool spawn_top = false;
        public bool spawn_bottom = false;

        void Start()
        {
            //spawn_timer = Time.GetElapsedTime() + spawn_time;
            spawn_timer = delay_initial_spawn_time;
        }

        void Update()
        {
            if (health <= 0.001f)
            {
                return;
            }

            float limit_spawn_left_direction = spawn_left ? -1.0f : 0.0f;
            float limit_spawn_right_direction = spawn_right ? 1.0f : 0.0f;
            float limit_spawn_top_direction = spawn_top ? 1.0f : 0.0f;
            float limit_spawn_bottom_direction = spawn_bottom ? -1.0f : 0.0f;

            float spawn_x_direction = random_generator.RandomFloat(limit_spawn_left_direction, limit_spawn_right_direction);
            float spawn_y_direction = random_generator.RandomFloat(limit_spawn_bottom_direction, limit_spawn_top_direction);

            Vector2 spawn_direction = new Vector2(spawn_x_direction, spawn_y_direction).Normalize();

            if (spawn_timer < Time.GetElapsedTime())
            {
                GameObject new_game_object = GameObject.CreateGameObject();
                new_game_object.AddComponent<Sprite>();
                new_game_object.transform.SetPosition(game_object.transform.GetPosition() + spawn_direction);
                new_game_object.transform.SetZIndex(1);
                PrefabSystem.InstanceUserPrefab(new_game_object, spawn_prefab_name);

                spawn_timer = Time.GetElapsedTime() + spawn_time;
            }
        }

        public override void TakeDamage(GameObject hit_object, float damage)
        {
            health -= damage;

            if (health <= 0.001f)
            {
                Render.LoadTexture("../QRGameEngine/Textures/Fence-4.png", game_object.GetComponent<Sprite>());
                return;
            }
            if (health <= 30.0f)
            {
                Render.LoadTexture("../QRGameEngine/Textures/Fence-3.png", game_object.GetComponent<Sprite>());
                return;
            }
            if (health <= 70.0f)
            {
                Render.LoadTexture("../QRGameEngine/Textures/Fence-2.png", game_object.GetComponent<Sprite>());
                return;
            }
            Render.LoadTexture("../QRGameEngine/Textures/Fence.png", game_object.GetComponent<Sprite>());
        }

        public override void Knockback(Vector2 dir, float knockback)
        {
            
        }
    }
}
