using ScriptProject.Scripts.Effects;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Scripts.Modules
{
    internal class EffectHolderModule
    {
        Effect effect;

        public bool IsEffectOver() { return effect == null || (effect != null && effect.IsEffectOver()); }

        public bool IsEffect<EffectType>() where EffectType : Effect { return effect != null && effect is EffectType; }

        public void SetEffect(InteractiveCharacterBehaviour character, Effect effect)
        {
            if (character.ShouldEffectBeSet(effect))
            {
                this.effect = effect;
            }
        }

        public Effect GetEffect() { return effect; }
    }
}
