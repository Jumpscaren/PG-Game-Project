using ScriptProject.Engine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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
        }

        void Update()
        {
            //Console.WriteLine(player_game_object);
            //Console.WriteLine(player_game_object.transform);
            game_object.transform.SetPosition(player_game_object.transform.GetPosition());
        }
    }
}
