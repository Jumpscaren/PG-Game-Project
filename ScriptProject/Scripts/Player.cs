using ScriptProject.Engine;
using ScriptProject.Math;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Scripts
{
    internal class Player : ScriptingBehaviour
    {
        UInt32 fisk;
        bool majs;
        double mj1;
        DynamicBody body;
        void Start()
        {
            body = game_object.GetComponent<DynamicBody>();
        }

        void Update()
        {
            float max_speed = 8.1f;
            Vector2 velocity = body.GetVelocity();
            Vector2 velocity_direction = velocity.Normalize();
            Vector2 new_velocity = new Vector2();
            if (Input.GetKeyDown(Input.Key.W))
                new_velocity.y += max_speed;
            if (Input.GetKeyDown(Input.Key.S))
                new_velocity.y -= max_speed;
            if (Input.GetKeyDown(Input.Key.D))
                new_velocity.x += max_speed;
            if (Input.GetKeyDown(Input.Key.A))
                new_velocity.x -= max_speed;

            new_velocity = new_velocity.Normalize() * max_speed;
            if (velocity.Length() <= max_speed && new_velocity.Length() != 0.0f)
                velocity = new_velocity;
            else
                velocity += new_velocity * Time.GetDeltaTime();
            if (new_velocity.Length() == 0.0f && velocity.Length() <= max_speed)
                velocity = new Vector2(0.0f, 0.0f);
            if (velocity.Length() > max_speed)
                velocity -= velocity.Normalize() * max_speed * 3.0f * Time.GetDeltaTime();
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
        }
    }
}
