using ScriptProject.Engine;
using ScriptProject.EngineMath;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Scripts
{
    internal class Shield : InteractiveCharacterBehaviour
    {
        const float TIME_BUFFER = 0.1f; 

        Dictionary<UInt64, float> hit_object_ids = new Dictionary<UInt64, float>();

        public bool HasHitObject(GameObject game_object) { return hit_object_ids.ContainsKey(game_object.GetGameObjectUID()); }

        public void ResetHitObjects() { 
            List<UInt64> hit_objects_to_remove = new List<UInt64>();
            foreach (var hit_object in hit_object_ids)
            {
                if (Time.GetElapsedTime() > hit_object.Value + TIME_BUFFER)
                {
                    hit_objects_to_remove.Add(hit_object.Key);
                }
            }

            foreach (UInt64 hit_object_id in hit_objects_to_remove)
            {
                //Console.WriteLine("Remove Hit Object Id = " + hit_object_id);
                hit_object_ids.Remove(hit_object_id);
            }
        }

        public override void TakeDamage(GameObject hit_object, float damage)
        {
            //Console.WriteLine("Hit Object: " + hit_object.GetName() + ", GUID = " + hit_object.GetGameObjectUID());
            hit_object_ids.Add(hit_object.GetGameObjectUID(), Time.GetElapsedTime());
        }

        public override void Knockback(Vector2 dir, float knockback)
        {
        }
    }
}
