using ScriptProject.Engine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ScriptProject.EngineMath;

namespace ScriptProject.Scripts
{
    internal class PlayerCamera : ScriptingBehaviour
    {
        GameObject player_game_object;

        void Start()
        {
            player_game_object = GameObject.TempFindGameObject("Player");
            Console.WriteLine("Player Name = " + player_game_object.GetName());
            game_object.transform.SetZIndex(20);
            game_object.transform.SetPosition(new Vector2(0, 0));
            player_game_object.AddChild(game_object);

            game_object.RemoveComponent<Sprite>();
        }
    }
}
