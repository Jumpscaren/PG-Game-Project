using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Scripts.Effects
{
    internal class StunEffect : Effect
    {
        public StunEffect(float duration) : base(duration)
        {
        }

        public override bool StopMovement() { return true; }
    }
}
