using ScriptProject.Engine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ScriptProject.EngineMath;
using ScriptProject.Engine.Constants;

namespace ScriptProject.Scripts
{
    internal class PlayerCamera : ScriptingBehaviour
    {
        GameObject player_game_object;
        DynamicBody player_body;

        Vector2 camera_velocity = new Vector2(0, 0);
        //const float camera_speed = 2.0f;
        //const float camera_speed = 2.0f;
        const float camera_speed = 4.0f;

        //const float time_behind_allowed = 0.05f;
        const float time_behind_allowed = 0.15f;

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

        void FixedUpdate()
        {
            //game_object.transform.SetPosition(Vector2.Lerp(game_object.transform.GetPosition(), player_game_object.transform.GetPosition(), 0.05f));

            float fixed_delta_time = PhysicConstants.TIME_STEP;

            Vector2 camera_to_player = player_game_object.transform.GetPosition() - game_object.transform.GetPosition();
            Vector2 player_velocity = player_body.GetVelocity();
            Vector2 interperated_player_speed = player_velocity;

            float f = interperated_player_speed.Length() * 0.9f * time_behind_allowed;

            Vector2 camera_to_player_with_speed = player_game_object.transform.GetPosition() + interperated_player_speed * fixed_delta_time - game_object.transform.GetPosition();

            float catch_up_speed = interperated_player_speed.Length() * camera_to_player.Length() / f;
            if (interperated_player_speed.Length() < 0.001f)
            {
                catch_up_speed = camera_speed;
            }

            camera_velocity = camera_to_player.Normalize() * catch_up_speed;
            Vector2 position = game_object.transform.GetPosition() + camera_velocity * fixed_delta_time;

            Vector2 new_position_to_camera = player_game_object.transform.GetPosition() - position;
            if (Vector2.DotProduct(new_position_to_camera.Normalize(), camera_to_player.Normalize()) > 0.0f)
            {
                game_object.transform.SetPosition(position);
            }
        }
    }
}
