using ScriptProject.Engine;
using ScriptProject.Engine.Constants;
using ScriptProject.EngineMath;
using ScriptProject.Scripts.Effects;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Scripts
{
    internal abstract class InteractiveCharacterBehaviour : ScriptingBehaviour
    {
        // Set to false if the character should avoid hitboxes
        public virtual bool Interactable() { return true; }

        public abstract void SetEffect(Effect effect);
        public virtual bool ShouldEffectBeSet(Effect effect) { return true; }

        public abstract void TakeDamage(GameObject hit_object, float damage);

        public abstract void Knockback(Vector2 dir, float knockback);
    }
}
