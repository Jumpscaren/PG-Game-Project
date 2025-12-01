using ScriptProject.Engine;
using ScriptProject.EngineMath;
using ScriptProject.EngineFramework;

namespace ScriptProject.Scripts
{
    internal class Hammer : ScriptingBehaviour
    {
        GameObject hit_box;
        const float destroy_time = 5.0f;
        Timer destroy_timer = new Timer(destroy_time);

        const float angle_change = 30.0f;
        float angle = 0.0f;

        float speed = 10.0f;

        public void InitHammer(Vector2 position, Vector2 direction, GameObject avoid_game_object)
        {
            Sprite arrow_sprite = game_object.AddComponent<Sprite>();
            Render.LoadTexture("../QRGameEngine/Textures/Hammer.png", arrow_sprite);
            game_object.transform.SetPosition(position);
            game_object.transform.SetScale(new Vector2(0.3f, 0.3f));
            game_object.AddComponent<KinematicBody>().SetVelocity(direction * speed);

            hit_box = GameObject.CreateGameObject();
            hit_box.AddComponent<StaticBody>();
            hit_box.AddComponent<BoxCollider>().SetTrigger(true);

            hit_box.SetName("HammerHitBox");

            HitBox hit_box_script = hit_box.AddComponent<HitBox>();
            hit_box_script.SetHitBoxAction(new HitBoxHammer(), game_object);
            hit_box_script.SetAvoidGameObject(avoid_game_object);

            game_object.AddChild(hit_box);

            destroy_timer.Start();
        }

        void FixedUpdate()
        {
            angle += angle_change * Time.GetFixedDeltaTime();
            game_object.transform.SetLocalRotation(angle);

            if (destroy_timer.IsExpired())
            {
                GameObject.DeleteGameObject(game_object);
            }
        }

        public class HitBoxHammer : HitBoxAction
        {
            float damage = 5.0f;
            float knockback = 5.3f;

            public override void OnHit(GameObject hit_box_owner_game_object, ScriptingBehaviour hit_box_script, InteractiveCharacterBehaviour hit_object_script)
            {
                if (!hit_box_script.GetGameOjbect().HasParent())
                {
                    return;
                }

                if (hit_object_script.GetGameOjbect() == hit_box_script.GetGameOjbect().GetParent()) return;

                Vector2 dir = hit_object_script.GetGameOjbect().transform.GetPosition() - hit_box_script.GetGameOjbect().transform.GetPosition();
                dir = dir.Normalize();

                hit_object_script.Knockback(dir, knockback);
                hit_object_script.TakeDamage(hit_box_script.GetGameOjbect(), damage);

                GameObject.DeleteGameObject(hit_box_script.GetGameOjbect().GetParent());
            }

            public override void OnHitAvoidGameObject(ScriptingBehaviour hit_box_script)
            {

            }
        }
    }
}
