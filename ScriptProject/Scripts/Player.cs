using ScriptProject.Engine;
using ScriptProject.EngineMath;
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
        bool attack = false;
        GameObject mid_block = null;
        GameObject hit_box;
        GameObject camera;
        void Start()
        {
            body = game_object.GetComponent<DynamicBody>();
            sprite = game_object.GetComponent<Sprite>();
            anim_sprite = game_object.GetComponent<AnimatableSprite>();
            hit_box = GameObject.CreateGameObject();
            hit_box.SetName("Attack_Box");
            hit_box.AddComponent<StaticBody>().SetEnabled(false);
            BoxCollider box_collider = hit_box.AddComponent<BoxCollider>();
            box_collider.SetTrigger(false);
            box_collider.SetHalfBoxSize(new Vector2(0.3f, 0.5f));
            hit_box.transform.SetPosition(1.0f, 0.0f);
            mid_block = GameObject.CreateGameObject();
            mid_block.AddChild(hit_box);
            game_object.AddChild(mid_block);

            camera = GameObject.TempFindGameObject("PlayerCamera");
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

            if (new_velocity.Length() < 0.01f && !attack)
            {
                if (!AnimationManager.IsAnimationPlaying(game_object, "Animations/KnightIdle.anim"))
                    AnimationManager.LoadAnimation(game_object, "Animations/KnightIdle.anim");
                //sprite.SetTexture(Render.LoadTexture("../QRGameEngine/Textures/Knight_Idle_Atlas.png"));
                //sprite.SetUV(new Vector2(0.024f, 0.220f), new Vector2(0.055f, 0.7f));
                //anim_sprite.SetSplitSize(new Vector2((960.0f / 15.0f) / 960.0f, 0));
                //anim_sprite.SetMaxSplits(15);
                //anim_sprite.SetTimeBetweenSplits(0.1f);
                //anim_sprite.SetLoop(true);
                //anim_sprite.SetId(2);
            }
            else if (!attack && !AnimationManager.IsAnimationPlaying(game_object, "Animations/KnightRunAnim.anim"))
            {
                AnimationManager.LoadAnimation(game_object, "Animations/KnightRunAnim.anim");
                //sprite.SetTexture(Render.LoadTexture("../QRGameEngine/Textures/Knight_Run_Atlas.png"));
                //sprite.SetUV(new Vector2(0.052f, 0.220f), new Vector2(0.095f, 0.7f));
                //anim_sprite.SetSplitSize(new Vector2((768.0f / 8.0f) / 768.0f, 0));
                //anim_sprite.SetMaxSplits(7);
                //anim_sprite.SetTimeBetweenSplits(0.1f);
                //anim_sprite.SetLoop(true);
                //anim_sprite.SetId(2);
            }

            if (Input.GetMouseButtonPressed(Input.MouseButton.LEFT))
            {
                AnimationManager.LoadAnimation(game_object, "Animations/KnightAttack.anim");
                //sprite.SetTexture(Render.LoadTexture("../QRGameEngine/Textures/Knight_Attack_Atlas_2.png"));
                //sprite.SetUV(new Vector2(0.06f, 0.220f), new Vector2(0.078f, 0.7f));
                //anim_sprite.SetSplitSize(new Vector2((3168 / 22.0f) / 3168.0f, 0));
                //anim_sprite.SetMaxSplits(21);
                //anim_sprite.SetTimeBetweenSplits(0.1f);
                //anim_sprite.SetLoop(false);
                //anim_sprite.ResetAnimation();
                //anim_sprite.SetId(3);
                attack = true;
                hit_box.GetComponent<StaticBody>().SetEnabled(true);
            }

            if (!AnimationManager.IsAnimationPlaying(game_object, "Animations/KnightAttack.anim"))
            {
                attack = false;
            }

            Vector2 mouse_position = Input.GetMousePositionInWorld(camera);
            Vector2 mouse_dir = (mouse_position - game_object.transform.GetPosition()).Normalize();
            Vector2 right_dir = new Vector2(1.0f, 0.0f);
            mid_block.transform.SetLocalRotation(Vector2.Angle(mouse_dir, right_dir));
            sprite.FlipX(mouse_dir.x < 0);

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
