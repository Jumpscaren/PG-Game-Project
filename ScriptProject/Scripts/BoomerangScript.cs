using ScriptProject.Engine;
using ScriptProject.EngineMath;
using System;

namespace ScriptProject.Scripts
{
    internal class BoomerangScript : ScriptingBehaviour
    {
        KinematicBody body;

        float alive_timer = 0.0f;
        float alive_time = 10.0f;

        float rotation = 0.0f;
        const float base_rotation = 30.0f;
        float rotate = base_rotation;
        const float decrease_rotate = 2.0f;
        const float min_rotate_before_returning = 6.0f;

        float wait_timer;
        float wait_time = 0.5f;

        float speed = 12.0f;

        GameObject hit_box;

        Vector2 anchor_position;

        const float outwards_scalar = -5.5f;

        bool return_to_owner = false;
        bool returned = false;
        GameObject owner;
        DynamicBody owner_body;

        RandomGenerator random_generator = new RandomGenerator();

        void Start()
        {
            body = game_object.AddComponent<KinematicBody>();
            CircleCollider circle_collider = game_object.AddComponent<CircleCollider>();
            circle_collider.SetTrigger(true);
            circle_collider.SetRadius(0.3f);
            alive_timer = Time.GetElapsedTime() + alive_time;
            anchor_position = game_object.transform.GetPosition();

            hit_box = GameObject.CreateGameObject();
            hit_box.AddComponent<StaticBody>();
            hit_box.AddComponent<CircleCollider>().SetTrigger(true);

            hit_box.SetName("BoomerangHitBox");

            HitBox hit_box_script = hit_box.AddComponent<HitBox>();
            hit_box_script.SetHitBoxAction(new HitBoxBoomerang());
            hit_box_script.SetAvoidGameObject(GameObject.TempFindGameObject("Player"));

            game_object.AddChild(hit_box);

            wait_timer = wait_time + Time.GetElapsedTime();
        }

        void Update()
        {
            float current_speed = speed * rotate / base_rotation;
            float current_outward_scalar = outwards_scalar + (float)Math.Sin((double)Time.GetElapsedTime()) * 2.0f;
            if (wait_timer < Time.GetElapsedTime())
            {
                if (!return_to_owner)
                {
                    Vector2 dir_to_anchor = (anchor_position - game_object.transform.GetPosition()).Normalize();
                    Vector2 velocity_dir = (dir_to_anchor.PerpendicularClockwise() -
                        dir_to_anchor * current_outward_scalar * Time.GetDeltaTime()).Normalize();
                    Vector2 velocity = velocity_dir * current_speed;
                    body.SetVelocity(velocity);
                }
                else
                {
                    Vector2 predicted_position = owner.transform.GetPosition() + owner_body.GetVelocity() * Time.GetDeltaTime();
                    body.SetVelocity((predicted_position - game_object.transform.GetPosition()).Normalize() * speed);
                    rotate = base_rotation;

                    if (returned)
                    {
                        GameObject.DeleteGameObject(game_object);
                    }
                }
            }

            rotation += rotate * Time.GetDeltaTime();
            rotate -= decrease_rotate * Time.GetDeltaTime();
            game_object.transform.SetLocalRotation(rotation);

            if (rotate < min_rotate_before_returning && !return_to_owner)
            {
                return_to_owner = true;
            }
        }

        void BeginCollision(GameObject collided_game_object)
        {
            if (collided_game_object == owner)
            {
                returned = true;
            }
        }

        void EndCollision(GameObject collided_game_object)
        {
            if (collided_game_object == owner)
            {
                returned = false;
            }
        }

        public BoomerangScript SetInitialDirection(Vector2 initial_direction)
        {
            body.SetVelocity(initial_direction * speed);
            return this;
        }

        public BoomerangScript SetOwner(GameObject in_owner)
        {
            owner = in_owner;
            owner_body = owner.GetComponent<DynamicBody>();
            return this;
        }

        public class HitBoxBoomerang : HitBoxAction
        {
            float damage = 20.0f;
            float knockback = 7.3f;

            public override void OnHit(ScriptingBehaviour hit_box_script, InteractiveCharacterBehaviour hit_object_script)
            {
                if (!hit_box_script.GetGameOjbect().HasParent())
                {
                    return;
                }

                if (hit_object_script is Princess)
                {
                    return;
                }

                if (hit_object_script.GetGameOjbect() == hit_box_script.GetGameOjbect().GetParent()) return;

                Vector2 dir = hit_object_script.GetGameOjbect().transform.GetPosition() - hit_box_script.GetGameOjbect().transform.GetPosition();
                dir = dir.Normalize();

                hit_object_script.Knockback(dir, knockback);
                hit_object_script.TakeDamage(hit_box_script.GetGameOjbect(), damage);
            }

            public override void OnHitAvoidGameObject(ScriptingBehaviour hit_box_script)
            {

            }
        }
    }
}
