using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ScriptProject.EngineFramework;

namespace ScriptProject.Scripts.Effects
{
    internal class Effect
    {
        Timer effect_timer;

        public Effect(float duration)
        {
            effect_timer = new Timer();
            effect_timer.SetTimeLimit(duration);
            effect_timer.Start();
        }

        public bool IsEffectOver() { return effect_timer.IsExpired(); }

        public virtual bool StopMovement() { return false; }
    }
}
