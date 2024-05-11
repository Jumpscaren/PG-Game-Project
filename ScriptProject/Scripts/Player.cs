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
        const float attack_time = 0.1f;
        float attack_timer = 0.0f;
        const float attack_angle = (float)Math.PI / 2.0f;
        const float between_attack_time = 0.45f;
        float between_attack_timer = 0.0f;

        float health = 100.0f;

        const float max_speed = 4.0f;//8.1f;
        const float princess_speed = max_speed * 0.7f;
        const float drag_speed = 20.0f;
        const float attack_speed = max_speed / 2.0f;

        GameObject princess;
        bool holding_princess = false;
        Princess princess_script = null;

        void Start()
        {
            body = game_object.GetComponent<DynamicBody>();
            sprite = game_object.GetComponent<Sprite>();
            anim_sprite = game_object.GetComponent<AnimatableSprite>();
            hit_box = GameObject.CreateGameObject();
            hit_box.SetName("Attack_Box");
            hit_box.AddComponent<StaticBody>().SetEnabled(false);
            BoxCollider box_collider = hit_box.AddComponent<BoxCollider>();
            box_collider.SetTrigger(true);
            box_collider.SetHalfBoxSize(new Vector2(0.6f, 0.5f));
            hit_box.transform.SetPosition(0.7f, 0.0f);
            hit_box.SetName("HitBox");
            mid_block = GameObject.CreateGameObject();
            mid_block.AddChild(hit_box);
            game_object.AddChild(mid_block);

            //{
            //    GameObject hit_box1 = GameObject.CreateGameObject();
            //    hit_box1.SetName("Attack_Box");
            //    hit_box1.AddComponent<StaticBody>().SetEnabled(false);
            //    box_collider = hit_box1.AddComponent<BoxCollider>();
            //    box_collider.SetTrigger(true);
            //    box_collider.SetHalfBoxSize(new Vector2(0.6f, 0.5f));
            //    hit_box1.transform.SetPosition(0.7f, 0.0f);
            //    hit_box1.SetName("HitBox1");
            //    mid_block.AddChild(hit_box1);
            //}

            camera = GameObject.TempFindGameObject("PlayerCamera");

            princess = GameObject.TempFindGameObject("Princess");
            princess_script = princess.GetComponent<Princess>();
        }

        void Update()
        {
            if (health <= 0.0f) { 
                health = 0.0f;
                Console.WriteLine("Player Restart");
                SceneManager.RestartActiveScene();
            }

            float current_speed = max_speed;
            if (holding_princess)
            {
                current_speed = princess_speed;
            }
            if (between_attack_timer >= Time.GetElapsedTime())
            {
                current_speed = attack_speed;
            }

            if (attack_timer < Time.GetElapsedTime())
            {
                hit_box.GetComponent<StaticBody>().SetEnabled(false);
            }

            Vector2 velocity = body.GetVelocity();
            Vector2 new_velocity = new Vector2();
            if (Input.GetKeyDown(Input.Key.W))
                new_velocity.y += max_speed;
            if (Input.GetKeyDown(Input.Key.S))
                new_velocity.y -= max_speed;
            if (Input.GetKeyDown(Input.Key.D))
            {
                new_velocity.x += max_speed;
            }
            if (Input.GetKeyDown(Input.Key.A))
            {
                new_velocity.x -= max_speed;
            }

            if (new_velocity.Length() < 0.01f && !attack)
            {
                if (!AnimationManager.IsAnimationPlaying(game_object, "Animations/KnightIdle.anim"))
                    AnimationManager.LoadAnimation(game_object, "Animations/KnightIdle.anim");
            }
            else if (!attack && !AnimationManager.IsAnimationPlaying(game_object, "Animations/KnightRunAnim.anim"))
            {
                AnimationManager.LoadAnimation(game_object, "Animations/KnightRunAnim.anim");
            }

            if (!holding_princess && Input.GetMouseButtonPressed(Input.MouseButton.LEFT) && between_attack_timer < Time.GetElapsedTime())
            {
                AnimationManager.LoadAnimation(game_object, "Animations/KnightAttack.anim");
                attack = true;
                hit_box.GetComponent<StaticBody>().SetEnabled(true);
                attack_timer = attack_time + Time.GetElapsedTime();
                between_attack_timer = attack_timer + between_attack_time;
            }

            if (!AnimationManager.IsAnimationPlaying(game_object, "Animations/KnightAttack.anim"))
            {
                attack = false;
            }

            Vector2 mouse_position = Input.GetMousePositionInWorld(camera);
            Vector2 mouse_dir = (mouse_position - game_object.transform.GetPosition()).Normalize();
            Vector2 right_dir = new Vector2(1.0f, 0.0f);
            float calculated_rot = Vector2.Angle(mouse_dir, right_dir);

            mid_block.transform.SetLocalRotation(GetMidBlockRotation(calculated_rot));
            sprite.FlipX(mouse_dir.x < 0);

            new_velocity = new_velocity.Normalize() * current_speed;
            if (velocity.Length() <= current_speed && new_velocity.Length() != 0.0f)
                velocity = new_velocity;
            //else
            //    velocity += new_velocity * Time.GetDeltaTime();
            if (new_velocity.Length() == 0.0f && velocity.Length() <= current_speed)
                velocity = new Vector2(0.0f, 0.0f);
            if (velocity.Length() > current_speed)
                velocity -= velocity.Normalize() * drag_speed * Time.GetDeltaTime();
            if (new_velocity.Length() > 0.0f && velocity.Length() < current_speed)
                velocity = velocity.Normalize() * current_speed;

            body.SetVelocity(velocity);

            PrincessLogic();
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
                body.SetVelocity(dir * 11.0f);

                PrincessStopFollowPlayer();
            }
        }

        float GetMidBlockRotation(float calculated_rot)
        {
            float attack_time_rot = 0.0f;
            if (attack_timer > Time.GetElapsedTime())
            {
                attack_time_rot = (1.0f - (attack_timer - Time.GetElapsedTime()) / attack_time) * attack_angle;
            }
            return calculated_rot - attack_angle / 2.0f + attack_time_rot;
        }


        void PrincessLogic()
        {
            float distance_from_princess = (game_object.transform.GetPosition() - princess.transform.GetPosition()).Length();
            princess_script.KnightHoldingPrincess(holding_princess);

            if (holding_princess && Input.GetKeyPressed(Input.Key.E))
            {
                holding_princess = false;
                return;
            }

            if (distance_from_princess > 1.5f)
            {
                return;
            }

            if (Input.GetKeyPressed(Input.Key.E))
            {
                holding_princess = true;
            }
        }

        public void PrincessStopFollowPlayer()
        {
            holding_princess = false;
            princess_script.KnightHoldingPrincess(holding_princess);
        }
    }
}
