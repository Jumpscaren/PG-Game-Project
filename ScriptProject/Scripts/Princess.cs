using ScriptProject.Engine;
using ScriptProject.EngineMath;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Scripts
{
    internal class Princess : ScriptingBehaviour
    {
        DynamicBody body;
        Sprite sprite;
        const float max_speed = 1.0f;
        const float drag_speed = 20.0f;
        const float start_to_move_time = 5;
        float start_to_move_timer = 0;
        const float new_random_direction_time = 3;
        float new_random_direction_timer = 0;
        Vector2 random_direction;
        RandomGenerator random_generator = new RandomGenerator();

        bool follow_player = false;
        DynamicBody player_body;
        GameObject player;
        Player player_script;
        const float max_distance_to_player = 0.75f;

        float health = 100.0f;

        void Start()
        {
            body = game_object.GetComponent<DynamicBody>();
            sprite = game_object.GetComponent<Sprite>();
            random_direction = new Vector2(0, 0);
            start_to_move_timer = Time.GetElapsedTime() + start_to_move_time;
            new_random_direction_timer = start_to_move_time + new_random_direction_time;

            player = GameObject.TempFindGameObject("Player");
            player_body = player.GetComponent<DynamicBody>();
            player_script = player.GetComponent<Player>();
        }

        void Update() 
        {
            if (health <= 1e-05)
            {
                health = 0.0f;
                Console.WriteLine("Princess Restart");
                SceneManager.RestartActiveScene();
            }

            if (follow_player)
            {
                start_to_move_timer = Time.GetElapsedTime() + start_to_move_time;
                new_random_direction_timer = start_to_move_timer + new_random_direction_time;

                Vector2 direction_to_player = player.transform.GetPosition() - game_object.transform.GetPosition();
                float distance_to_player = direction_to_player.Length();
                if (distance_to_player > max_distance_to_player)
                {
                    game_object.transform.SetPosition(player.transform.GetPosition() - direction_to_player.Normalize() * max_distance_to_player);
                }

                var player_velocity = player_body.GetVelocity();
                //body.SetVelocity(player_velocity);
                sprite.FlipX(player_velocity.x < 0);
                return;
            }

            if (start_to_move_timer >= Time.GetElapsedTime())
            {
                random_direction = new Vector2(0, 0);
            }

            if (new_random_direction_timer < Time.GetElapsedTime())
            {
                random_direction = new Vector2(random_generator.RandomFloat(-1.0f, 1.0f), random_generator.RandomFloat(-1.0f, 1.0f));
                new_random_direction_timer = Time.GetElapsedTime() + new_random_direction_time;
            }

            Vector2 velocity = body.GetVelocity();
            Vector2 new_velocity = random_direction;

            sprite.FlipX(new_velocity.x < 0);

            new_velocity = new_velocity.Normalize() * max_speed;
            if (velocity.Length() <= max_speed && new_velocity.Length() != 0.0f)
                velocity = new_velocity;
            //else
            //    velocity += new_velocity * Time.GetDeltaTime();
            if (new_velocity.Length() == 0.0f && velocity.Length() <= max_speed)
                velocity = new Vector2(0.0f, 0.0f);
            if (velocity.Length() > max_speed)
                velocity -= velocity.Normalize() * drag_speed * Time.GetDeltaTime();
            if (new_velocity.Length() > 0.0f && velocity.Length() < max_speed)
                velocity = velocity.Normalize() * max_speed;

            body.SetVelocity(velocity);
        }


        void BeginCollision(GameObject collided_game_object)
        {
            if (collided_game_object.GetName() == "Bouncer")
            {
                Vector2 direction = game_object.transform.GetPosition() - collided_game_object.transform.GetPosition();
                body.SetVelocity(direction.Normalize() * 20.0f);
            }

            if (collided_game_object.GetName() == "OrcHitBox")
            {
                health -= 20.0f;
                float rot = collided_game_object.GetParent().transform.GetLocalRotation();
                Vector2 dir = new Vector2((float)Math.Cos(rot), (float)Math.Sin(rot));
                body.SetVelocity(dir * 10.3f);
                follow_player = false;
                player_script.PrincessStopFollowPlayer();
                OrcEnemy.OrcAngryEventData event_data = new OrcEnemy.OrcAngryEventData();
                event_data.orc_to_target = collided_game_object.GetParent().GetParent();
                EventSystem.SendEvent("OrcAngry", event_data);
            }
        }

        public void KnightHoldingPrincess(bool holding)
        {            
            follow_player = holding;
        }
    }
}
