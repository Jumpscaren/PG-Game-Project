using ScriptProject.Engine;
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
        void Start()
        {

        }

        void Update()
        {
            Vector2 pos = game_object.transform.GetPosition();

            if (Input.GetKeyDown(Input.Key.D))
                pos.x += 0.01f;

            if (Input.GetKeyDown(Input.Key.A))
                pos.x -= 0.01f;

            if (Input.GetKeyDown(Input.Key.W))
                pos.y += 0.01f;

            if (Input.GetKeyDown(Input.Key.S))
                pos.y -= 0.01f;

            if (Input.GetKeyDown(Input.Key.R) || Input.GetMouseButtonPressed(Input.MouseButton.WHEEL))
            {
                pos.x = 0;
                pos.y = 0;
            }

            if (Input.GetMouseButtonPressed(Input.MouseButton.LEFT))
                pos.y -= 0.02f;

            if (Input.GetMouseButtonPressed(Input.MouseButton.RIGHT))
                pos.y += 0.02f;

            game_object.transform.SetPosition(pos);

            //x += 0.01f;
            //game_object.transform.SetPosition(x, 0);
        }
    }
}
