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
            x += 0.01f;
            game_object.transform.SetPosition(x, 0 ,0);
        }
    }
}
