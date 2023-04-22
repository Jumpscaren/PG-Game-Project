using ScriptProject.Math;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject
{
    internal class TestScript : Engine.ScriptingBehaviour
    {
        float x = 0;

        void Start()
        {

        }

        void Update()
        {
            Vector2 pos = game_object.transform.GetPosition();

            pos.x += 0.01f;

            game_object.transform.SetPosition(pos);

            //x += 0.01f;
            //game_object.transform.SetPosition(x, 0);
        }
    }
}
