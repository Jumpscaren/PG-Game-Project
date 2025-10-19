using ScriptProject.Engine;
using ScriptProject.EngineMath;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Scripts
{
    internal class BasicEnemy : ScriptingBehaviour
    {
        GameObject player_game_object;
        PathFindingActor actor;
        Transform transform;
        static int enemy_count = 0;
        Vector2 last_position;
        DynamicBody body;
        float drag_speed = 20.0f;
        void Start()
        {
            player_game_object = GameObject.TempFindGameObject("Player");
            actor = game_object.GetComponent<PathFindingActor>();
            transform = game_object.transform;
            ++enemy_count;
            Console.WriteLine(enemy_count);
            last_position = transform.GetPosition();

            body = game_object.GetComponent<DynamicBody>();
        }

        void Update()
        {
            Vector2 current_position = transform.GetPosition();
            Vector2 dir = last_position - current_position;
            //last_position = actor.PathFind(player_game_object, 1);

            DynamicBody body = game_object.GetComponent<DynamicBody>();
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
                velocity -= velocity.Normalize() * drag_speed * Time.GetDeltaTime();
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

            if (collided_game_object.GetName() == "HitBox")
            {
                float rot = collided_game_object.GetParent().transform.GetLocalRotation();
                Vector2 dir = new Vector2((float)Math.Cos(rot), (float)Math.Sin(rot));
                body.SetVelocity(dir * 15.0f);
            }
        }
    }
}
