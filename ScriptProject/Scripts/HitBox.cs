using ScriptProject.Engine;
using System;

namespace ScriptProject.Scripts
{
    internal abstract class HitBoxAction
    {
        public abstract void OnHit(GameObject hit_box_owner_game_object, ScriptingBehaviour hit_box_script, InteractiveCharacterBehaviour hit_object_script);
        public abstract void OnHitAvoidGameObject(ScriptingBehaviour hit_box_script);
    }

    internal class HitBox : ScriptingBehaviour
    {
        HitBoxAction hit_box_action = null;
        GameObject avoid_game_object = null;
        GameObject owner_game_object = null;

        public void SetHitBoxAction(HitBoxAction in_hit_box_action, GameObject owner)
        {
            hit_box_action = in_hit_box_action;
            owner_game_object = owner;
        }

        public void SetAvoidGameObject(GameObject avoid)
        {
            avoid_game_object = avoid;
        }

        public void OnHit(InteractiveCharacterBehaviour hit_script)
        {
            if (hit_box_action == null)
            {
                Console.WriteLine("No Hit Box Action!");
                return;
            }

            if (!hit_script.Interactable())
            {
                return;
            }

            hit_box_action.OnHit(owner_game_object, this, hit_script);
        }

        public void BeginCollision(GameObject collided_game_object)
        {
            //Console.WriteLine("Begin Collision: " + game_object.GetName());
            Console.WriteLine("HitBox entity: " + game_object.GetEntityID());

            if (collided_game_object == avoid_game_object || !collided_game_object.HasComponent<ScriptingBehaviour>())
            {
                hit_box_action.OnHitAvoidGameObject(this);
                return;
            }

            ScriptingBehaviour script = collided_game_object.GetComponent<ScriptingBehaviour>();
            if (typeof(OrcShield).IsAssignableFrom(script.GetType()))
            {
                Console.WriteLine(game_object.GetName() + " Collided with " + collided_game_object.GetName());
            }
            if (typeof(InteractiveCharacterBehaviour).IsAssignableFrom(script.GetType()))
            {
                OnHit((InteractiveCharacterBehaviour)script);
            }
        }

        public void EndCollision(GameObject collided_game_object)
        {
            ScriptingBehaviour script = collided_game_object.GetComponent<ScriptingBehaviour>();
            if (typeof(OrcShield).IsAssignableFrom(script.GetType()))
            {
                Console.WriteLine(game_object.GetName() + " Stopped Collided with " + collided_game_object.GetName());
            }
        }
    }
}
