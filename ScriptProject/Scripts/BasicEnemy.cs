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
        void Start()
        {
            player_game_object = GameObject.TempFindGameObject("Player");
            actor = game_object.GetComponent<PathFindingActor>();
            transform = game_object.transform;
            ++enemy_count;
            Console.WriteLine(enemy_count);
            last_position = transform.GetPosition();
        }

        void Update()
        {
            Vector2 current_position = transform.GetPosition();
            Vector2 dir = last_position - current_position;
            last_position = actor.PathFind(player_game_object, 1);

            DynamicBody body = game_object.GetComponent<DynamicBody>();
            Vector2 velocity = body.GetVelocity();
            velocity += dir.Normalize() * Time.GetDeltaTime();
            body.SetVelocity(velocity);

            //Vector2 new_position = current_position + dir.Normalize() * Time.GetDeltaTime();
            //transform.SetPosition(new_position);
        }
    }
}
