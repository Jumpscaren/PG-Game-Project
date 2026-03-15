using ScriptProject.Engine;
using ScriptProject.EngineMath;
using ScriptProject.Scripts.Effects;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static System.Net.Mime.MediaTypeNames;

namespace ScriptProject.Scripts
{
    internal class PlayerDamageHitBox : InteractiveCharacterBehaviour
    {
        GameObject owner;
        Player player_script;

        KinematicBody damage_hit_box_body;
        CircleCollider damage_hit_box_collider;
        const float damage_hit_box_radius = 0.5f;

        public void Init(GameObject owner)
        {
            damage_hit_box_body = game_object.AddComponent<KinematicBody>();
            damage_hit_box_collider = game_object.AddComponent<CircleCollider>();
            damage_hit_box_collider.SetRadius(damage_hit_box_radius);
            damage_hit_box_collider.SetTrigger(true);

            player_script = owner.GetComponent<Player>();
        }

        public override void SetEffect(Effect effect) { player_script.SetEffect(effect); }

        public override bool ShouldEffectBeSet(Effect effect) { return player_script.ShouldEffectBeSet(effect); }

        public override void TakeDamage(GameObject hit_object, float damage)
        {
            player_script.TakeDamage(hit_object, damage);
        }

        public override void Knockback(Vector2 dir, float knockback)
        {
            player_script.Knockback(dir, knockback);
        }

        public void ActivateHitBox()
        {
            damage_hit_box_body.SetEnabled(true);
        }

        public void DeactivateHitBox()
        {
            damage_hit_box_body.SetEnabled(false);
        }

        public bool IsHitBoxActive() { return damage_hit_box_body.GetEnabled(); }
    }
}
