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
    internal class OrcEnemy : ScriptingBehaviour
    {
        GameObject player_game_object;
        PathFindingActor actor;
        Transform transform;
        Vector2 last_position;
        GameObject hit_box;
        GameObject mid_block;
        DynamicBody body;
        Sprite sprite;
        float health = 20.0f;

        void Start()
        {
            player_game_object = GameObject.TempFindGameObject("Player");
            actor = game_object.GetComponent<PathFindingActor>();
            transform = game_object.transform;
            last_position = transform.GetPosition();
            body = game_object.GetComponent<DynamicBody>();
            sprite = game_object.GetComponent<Sprite>();

            CreateHitBox();
        }

        void Update()
        {
            if (health <= 0.0f)
            {
                health = 0.0f;
                GameObject.DeleteGameObject(game_object);
                GameObject new_game_object = GameObject.CreateGameObject();
                new_game_object.AddComponent<Sprite>();
                PrefabSystem.InstanceUserPrefab(new_game_object, "OrcEnemy");
                return;
            }

            Look();
            Move();
        }

        void BeginCollision(GameObject collided_game_object)
        {
            if (collided_game_object.GetName() == "Bouncer")
            {
                DynamicBody body = game_object.GetComponent<DynamicBody>();
                Vector2 direction = game_object.transform.GetPosition() - collided_game_object.transform.GetPosition();
                body.SetVelocity(direction.Normalize() * 20.0f);
            }

            if (collided_game_object.GetName() == "HitBox")
            {
                DynamicBody body = game_object.GetComponent<DynamicBody>();
                Vector2 direction = game_object.transform.GetPosition() - collided_game_object.transform.GetPosition();
                body.SetVelocity(direction.Normalize() * 10.0f);
                health -= 10.0f;
            }
        }

        void CreateHitBox()
        {
            hit_box = GameObject.CreateGameObject();
            hit_box.SetName("Attack_Box");
            hit_box.AddComponent<StaticBody>().SetEnabled(true);
            BoxCollider box_collider = hit_box.AddComponent<BoxCollider>();
            box_collider.SetTrigger(true);
            box_collider.SetHalfBoxSize(new Vector2(0.3f, 0.5f));
            hit_box.transform.SetPosition(1.0f, 0.0f);
            hit_box.SetName("OrcHitBox");
            mid_block = GameObject.CreateGameObject();
            mid_block.AddChild(hit_box);
            game_object.AddChild(mid_block);
        }

        Vector2 right_dir = new Vector2(1.0f, 0.0f);
        void Look()
        {
            Vector2 player_position = player_game_object.transform.GetPosition();
            Vector2 player_dir = (player_position - game_object.transform.GetPosition()).Normalize();
            mid_block.transform.SetLocalRotation(Vector2.Angle(player_dir, right_dir));
            sprite.FlipX(player_dir.x < 0);
        }

        void Move()
        {
            Vector2 current_position = transform.GetPosition();
            Vector2 dir = last_position - current_position;
            last_position = actor.PathFind(player_game_object, 1);

            Vector2 velocity = body.GetVelocity();
            const float max_speed = 2.0f;

            Vector2 new_velocity = dir.Normalize() * max_speed;
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
    }
}
