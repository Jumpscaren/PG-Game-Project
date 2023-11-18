using ScriptProject.Engine;
using ScriptProject.Math;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Scripts
{
    internal class Player : ScriptingBehaviour
    {
        DynamicBody body;
        Sprite sprite;
        AnimatableSprite anim_sprite;
        void Start()
        {
            body = game_object.GetComponent<DynamicBody>();
            sprite = game_object.GetComponent<Sprite>();
            anim_sprite = game_object.GetComponent<AnimatableSprite>();
        }

        void Update()
        {
            float max_speed = 8.1f;
            Vector2 velocity = body.GetVelocity();
            Vector2 velocity_direction = velocity.Normalize();
            Vector2 new_velocity = new Vector2();
            bool flip_x = sprite.GetFlipX();
            if (Input.GetKeyDown(Input.Key.W))
                new_velocity.y += max_speed;
            if (Input.GetKeyDown(Input.Key.S))
                new_velocity.y -= max_speed;
            if (Input.GetKeyDown(Input.Key.D))
            {
                new_velocity.x += max_speed;
                flip_x = false;
            }
            if (Input.GetKeyDown(Input.Key.A))
            {
                new_velocity.x -= max_speed;
                flip_x = true;
            }
            if (new_velocity.Length() < 0.01f)
            {
                sprite.SetTexture(Render.LoadTexture("../QRGameEngine/Textures/Knight_Idle_Atlas.png"));
                sprite.SetUV(new Vector2(0.024f, 0.220f), new Vector2(0.055f, 0.7f));
                anim_sprite.SetSplitSize(new Vector2((960.0f / 15.0f) / 960.0f, 0));
                anim_sprite.SetMaxSplits(15);
                anim_sprite.SetTimeBetweenSplits(0.1f);
            }
            else
            {
                sprite.SetTexture(Render.LoadTexture("../QRGameEngine/Textures/Knight_Run_Atlas.png"));
                sprite.SetUV(new Vector2(0.052f, 0.220f), new Vector2(0.095f, 0.7f));
                anim_sprite.SetSplitSize(new Vector2((768.0f / 8.0f) / 768.0f, 0));
                anim_sprite.SetMaxSplits(7);
                anim_sprite.SetTimeBetweenSplits(0.1f);
            }
            sprite.FlipX(flip_x);

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
