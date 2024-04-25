using ScriptProject.Engine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Scripts
{
    internal class HitBox : ScriptingBehaviour
    {
        string hit_name = "";
        bool hit_specified = false;

        public void SetHitName(string name)
        {
            hit_name = name;
        }

        public void ResetHit()
        {
            hit_specified = false;
        }

        public bool HasHitSpecified()
        {
            return hit_specified;
        }

        void BeginCollision(GameObject collided_game_object)
        {
            if (hit_name == collided_game_object.GetName())
            {
                hit_specified = true;
            }
        }

        void EndCollision(GameObject collided_game_object)
        {
            if (hit_name == collided_game_object.GetName())
            {
                hit_specified = false;
            }
        }
    }
}
