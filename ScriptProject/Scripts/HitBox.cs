using ScriptProject.Engine;
using ScriptProject.EngineMath;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Scripts
{
    internal abstract class HitBoxAction
    {
        public abstract void OnHit(ScriptingBehaviour hit_box_script, InteractiveCharacterInterface hit_object_script);
    }

    internal class HitBox : ScriptingBehaviour
    {
        HitBoxAction hit_box_action = null;
        GameObject avoid_game_object = null;

        public void SetHitBoxAction(HitBoxAction in_hit_box_action)
        {
            hit_box_action = in_hit_box_action;
        }

        public void SetAvoidGameObject(GameObject avoid)
        {
            avoid_game_object = avoid;
        }

        public void OnHit(InteractiveCharacterInterface hit_script)
        {
            if (hit_box_action == null)
            {
                Console.WriteLine("No Hit Box Action!");
                return;
            }

            hit_box_action.OnHit(this, hit_script);
        }

        public void BeginCollision(GameObject collided_game_object, Vector2 normal)
        {
            if (collided_game_object == avoid_game_object || !collided_game_object.HasComponent<ScriptingBehaviour>())
            {
                return;
            }

            ScriptingBehaviour script = collided_game_object.GetComponent<ScriptingBehaviour>();
            if (typeof(InteractiveCharacterInterface).IsAssignableFrom(script.GetType()))
            {
                OnHit((InteractiveCharacterInterface)script);
            }
        }
    }
}
