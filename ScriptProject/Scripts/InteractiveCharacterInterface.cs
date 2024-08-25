using ScriptProject.Engine;
using ScriptProject.EngineMath;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Scripts
{
    internal abstract class InteractiveCharacterInterface : ScriptingBehaviour
    {
        public abstract void TakeDamage(GameObject hit_object, float damage);

        public abstract void Knockback(Vector2 dir, float knockback);
    }
}
