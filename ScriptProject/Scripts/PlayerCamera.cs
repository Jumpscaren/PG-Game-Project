using ScriptProject.Engine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ScriptProject.EngineMath;

namespace ScriptProject.Scripts
{
    internal class PlayerCamera : ScriptingBehaviour
    {
        GameObject player_game_object;
        DynamicBody player_body;

        Vector2 camera_velocity = new Vector2(0, 0);
        //const float camera_speed = 2.0f;
        const float camera_speed = 2.0f;

        //const float time_behind_allowed = 0.05f;
        const float time_behind_allowed = 0.1f;

        void Start()
        {
            player_game_object = GameObject.TempFindGameObject("Player");
            Console.WriteLine("Player Name = " + player_game_object.GetName());
            game_object.transform.SetLocalZIndex(20);
            game_object.transform.SetPosition(player_game_object.transform.GetPosition());
            //game_object.transform.SetLocalPosition(new Vector2(0, 0));

            game_object.RemoveComponent<Sprite>();

            player_body = player_game_object.GetComponent<DynamicBody>();
        }

        //void FixedUpdate(float dt)
        void LateUpdate()
        {
            float delta_time = Time.GetDeltaTime();

            Vector2 camera_to_player = player_game_object.transform.GetPosition() - game_object.transform.GetPosition();
            //Console.WriteLine("Camera to Player = " + camera_to_player);
            Vector2 player_velocity = player_body.GetVelocity();

            Vector2 interperated_player_speed = player_velocity;

            //float f = catch_up_speed * 0.9f * time_behind_allowed;
            float f = interperated_player_speed.Length() * 0.9f * time_behind_allowed;
            //Console.WriteLine("F = " + f);

            Vector2 camera_to_player_with_speed = player_game_object.transform.GetPosition() + interperated_player_speed * delta_time - game_object.transform.GetPosition();
            //Console.WriteLine("Camera to Player: " + camera_to_player.Length());
            //Console.WriteLine("F-: " + f * 0.95f);
            //Console.WriteLine("F+: " + f * 1.2f);
            float catch_up_speed = interperated_player_speed.Length() * camera_to_player.Length() / f;
            if (interperated_player_speed.Length() < 0.001f)
            {
                catch_up_speed = camera_speed;
            }
            camera_velocity = camera_to_player.Normalize() * catch_up_speed;
            Vector2 position = game_object.transform.GetPosition() + camera_velocity * delta_time;
            if ((camera_to_player.Length() > f * 0.95f && camera_to_player.Length() < f * 1.2f))
            {
                position = player_game_object.transform.GetPosition() - camera_to_player.Normalize() * (f);
            }

            Vector2 new_position_to_camera = player_game_object.transform.GetPosition() - position;
            if (Vector2.DotProduct(new_position_to_camera.Normalize(), camera_to_player.Normalize()) > 0.0f)
            {
                game_object.transform.SetPosition(position);
            }
        }
    }
}
