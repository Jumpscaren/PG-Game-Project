using ScriptProject.EngineMath;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Scripts
{
    internal abstract class ReactHitBox : HitBox
    {
        public abstract void Knockback(Vector2 dir, float knockback);
    }
}
