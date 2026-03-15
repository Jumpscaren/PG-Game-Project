using ScriptProject.Engine;
using ScriptProject.Engine.Constants;
using ScriptProject.EngineMath;
using ScriptProject.Scripts.Effects;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Scripts.Modules
{
    internal class CharacterMovementModule
    {
        static private void Movement(Vector2 velocity, Vector2 new_velocity, float max_speed, float drag_speed, DynamicBody body, EffectHolderModule effect, float delta_time)
        {
            if (!effect.IsEffectOver() && effect.IsEffect<StunEffect>())
            {
                max_speed = 0.0f;
            }

            const float epsilion = 0.0001f;

            float new_velocity_length = new_velocity.Length();
            if (velocity.Length() <= max_speed + epsilion && new_velocity_length > epsilion)
                velocity = new_velocity;
            //else
            //    velocity += new_velocity * Time.GetDeltaTime();
            if (new_velocity_length < epsilion && velocity.Length() <= max_speed + epsilion)
                velocity = new Vector2(0.0f, 0.0f);
            if (velocity.Length() > max_speed + epsilion)
                velocity -= velocity.Normalize() * drag_speed * delta_time;
            if (new_velocity_length > epsilion && velocity.Length() < max_speed + epsilion)
                velocity = velocity.Normalize() * max_speed;

            body.SetVelocity(velocity);
        }

        static public void VariedMovement(Vector2 new_velocity, float max_speed, float drag_speed, DynamicBody body, EffectHolderModule effect)
        {
            Movement(body.GetVelocity(), new_velocity, max_speed, drag_speed, body, effect, Time.GetDeltaTime());
        }

        static public void FixedMovement(Vector2 new_velocity, float max_speed, float drag_speed, DynamicBody body, EffectHolderModule effect)
        {
            Movement(body.GetVelocity(), new_velocity, max_speed, drag_speed, body, effect, PhysicConstants.TIME_STEP);
        }
    }
}
