using ScriptProject.Engine;
using ScriptProject.EngineMath;

namespace ScriptProject.Scripts
{
    internal class Arrow : ScriptingBehaviour
    {
        GameObject hit_box;
        float destroy_time = 5.0f;
        float destroy_timer = 0.0f;

        float speed = 20.0f;

        public void InitArrow(Vector2 position, Vector2 direction)
        {
            Sprite arrow_sprite = game_object.AddComponent<Sprite>();
            Render.LoadTexture("../QRGameEngine/Textures/Arrow.png", arrow_sprite);
            game_object.transform.SetPosition(position);
            game_object.transform.SetScale(new Vector2(1.0f, 0.5f) * 0.5f);
            game_object.transform.SetLocalRotation(Vector2.Angle(direction, new Vector2(1, 0)));
            game_object.AddComponent<KinematicBody>().SetVelocity(direction * speed);

            hit_box = GameObject.CreateGameObject();
            hit_box.AddComponent<StaticBody>();
            hit_box.AddComponent<BoxCollider>().SetTrigger(true);

            hit_box.SetName("ArrowHitBox");

            HitBox hit_box_script = hit_box.AddComponent<HitBox>();
            hit_box_script.SetHitBoxAction(new HitBoxArrow());
            hit_box_script.SetAvoidGameObject(GameObject.TempFindGameObject("Player"));

            game_object.AddChild(hit_box);

            destroy_timer = destroy_time + Time.GetElapsedTime();
        }

        void Update()
        {
            if (destroy_timer < Time.GetElapsedTime())
            {
                GameObject.DeleteGameObject(game_object);
            }
        }

        public class HitBoxArrow : HitBoxAction
        {
            float damage = 5.0f;
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

                hit_object_script.TakeDamage(hit_box_script.GetGameOjbect(), damage);

                Vector2 dir = hit_object_script.GetGameOjbect().transform.GetPosition() - hit_box_script.GetGameOjbect().transform.GetPosition();
                dir = dir.Normalize();

                hit_object_script.Knockback(dir, knockback);

                GameObject.DeleteGameObject(hit_box_script.GetGameOjbect().GetParent());
            }

            public override void OnHitAvoidGameObject(ScriptingBehaviour hit_box_script)
            {

            }
        }
    }
}
