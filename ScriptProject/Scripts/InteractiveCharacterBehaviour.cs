using ScriptProject.Engine;
using ScriptProject.EngineMath;
using ScriptProject.Scripts.Effects;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Scripts
{
    internal class InteractiveCharacterBehaviour : ScriptingBehaviour
    {
        Effect effect;

        public bool IsEffectOver() { return effect == null || (effect != null && effect.IsEffectOver()); }

        public void SetEffect(Effect effect) { this.effect = effect; }

        public Effect GetEffect() { return effect; }

        public void Movement(Vector2 velocity, Vector2 new_velocity, float max_speed, float drag_speed, DynamicBody body)
        {
            if (!IsEffectOver() && effect.StopMovement())
            {
                max_speed = 0.0f;
            }

            const float epsilion = 0.0001f;

            float new_velocity_length = new_velocity.Length();
            if (velocity.Length() <= max_speed && new_velocity_length > epsilion)
                velocity = new_velocity;
            //else
            //    velocity += new_velocity * Time.GetDeltaTime();
            if (new_velocity_length < epsilion && velocity.Length() <= max_speed)
                velocity = new Vector2(0.0f, 0.0f);
            if (velocity.Length() > max_speed)
                velocity -= velocity.Normalize() * drag_speed * Time.GetDeltaTime();
            if (new_velocity_length > epsilion && velocity.Length() < max_speed)
                velocity = velocity.Normalize() * max_speed;

            body.SetVelocity(velocity);
        }

        public virtual void TakeDamage(GameObject hit_object, float damage)
        {
            Console.Error.WriteLine("TakeDamage not implemented");
        }

        public virtual void Knockback(Vector2 dir, float knockback)
        {
            Console.Error.WriteLine("Knockback not implemented");
        }
    }
}
