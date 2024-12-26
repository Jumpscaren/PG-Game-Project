using ScriptProject.Engine;
using ScriptProject.EngineMath;
using System;

namespace ScriptProject.Scripts
{
    internal class ProjectileScript : InteractiveCharacterInterface
    {
        DynamicBody body;
        const float max_speed = 7.0f;
        float speed = max_speed;
        GameObject player;
        DynamicBody player_body;
        float alive_timer = 0.0f;
        float alive_time = 10.0f;

        GameObject hit_box;

        bool has_been_hit = false;

        void Start()
        {
            body = game_object.GetComponent<DynamicBody>();
            player = GameObject.TempFindGameObject("Player");
            player_body = player.GetComponent<DynamicBody>();

            Vector2 predicted_position = player.transform.GetPosition() + player_body.GetVelocity() * 1.0f;
            body.SetVelocity((predicted_position - game_object.transform.GetPosition()).Normalize() * max_speed);
            alive_timer = Time.GetElapsedTime() + alive_time;


            hit_box = GameObject.CreateGameObject();
            hit_box.AddComponent<StaticBody>();
            hit_box.AddComponent<CircleCollider>().SetTrigger(true);

            hit_box.SetName("FireballHitBox");

            HitBox hit_box_script = hit_box.AddComponent<HitBox>();
            hit_box_script.SetHitBoxAction(new HitBoxProjectile());

            game_object.AddChild(hit_box);
        }

        void Update()
        {
            if (alive_timer < Time.GetElapsedTime())
            {
                GameObject.DeleteGameObject(game_object);
            }

            if (has_been_hit)
            {
                return;
            }

            speed = max_speed * (alive_timer - Time.GetElapsedTime()) / alive_time + 0.1f;

            Vector2 velocity = body.GetVelocity();
            Vector2 predicted_position = player.transform.GetPosition() + player_body.GetVelocity() * 0.2f;
            Vector2 direction = predicted_position - game_object.transform.GetPosition();
            Vector2 new_velocity = velocity + direction.Normalize() * speed * Time.GetDeltaTime();
            if (new_velocity.Length() > speed)
            {
                new_velocity = new_velocity.Normalize() * speed;
            }
            body.SetVelocity(new_velocity);
        }

        public override void TakeDamage(GameObject hit_object, float damage)
        {
        }

        public override void Knockback(Vector2 dir, float knockback)
        {
            body.SetVelocity(dir * 10.0f);
            has_been_hit = true;
        }

        public void SetCreator(GameObject creator)
        {
            Console.WriteLine("Creator: " + creator);
            hit_box.GetComponent<HitBox>().SetAvoidGameObject(creator);
        }

        public class HitBoxProjectile : HitBoxAction
        {
            float damage = 20.0f;
            float knockback = 7.3f;

            public override void OnHit(ScriptingBehaviour hit_box_script, InteractiveCharacterInterface hit_object_script)
            {
                if (!hit_box_script.GetGameOjbect().HasParent())
                {
                    return;
                }

                if (hit_object_script.GetGameOjbect() == hit_box_script.GetGameOjbect().GetParent()) return;

                hit_object_script.TakeDamage(hit_box_script.GetGameOjbect(), damage);

                Vector2 dir = hit_object_script.GetGameOjbect().transform.GetPosition() - hit_box_script.GetGameOjbect().transform.GetPosition();
                dir = dir.Normalize();

                hit_object_script.Knockback(dir, knockback);

                GameObject.DeleteGameObject(hit_box_script.GetGameOjbect().GetParent());
            }

            public override void OnHitAvoidGameObject(ScriptingBehaviour hit_box_script)
            {
                hit_box_script.GetGameOjbect().GetParent().GetComponent<ProjectileScript>().SetCreator(null);
            }
        }
    }
}
