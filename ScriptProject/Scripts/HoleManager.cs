using ScriptProject.Engine;
using ScriptProject.UserDefined;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Scripts
{
    internal class HoleManager
    {
        private List<GameObject> holes = new List<GameObject>();

        //Returns Hole
        public GameObject AddHole(GameObject collided_game_object, bool dead, float max_speed, float current_speed)
        {
            if (!dead && collided_game_object.GetTag() == UserTags.Hole)
            {
                if (current_speed > max_speed)
                {
                    holes.Add(collided_game_object);
                    return collided_game_object;
                }
                else
                {
                    return null;
                }
            }

            return collided_game_object;
        }

        public void RemoveHole(GameObject collided_game_object)
        {
            if (collided_game_object.GetTag() == UserTags.Hole)
            {
                holes.Remove(collided_game_object);
            }
        }

        public bool ShouldDieInHole(float current_speed, float max_speed)
        {
            return holes.Count > 0 && current_speed - 0.1001f <= max_speed;
        }

        public bool InHoles()
        {
            return holes.Count > 0;
        }
    }
}
