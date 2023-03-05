using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class GameObject
    {
        UInt32 entity_id;

        static public GameObject CreateGameObject()
        {
            GameObject gameObject = new GameObject();
            gameObject.entity_id = SceneManager.GetActiveScene().GetEntityManager().NewEntity();
            Console.WriteLine("GameObject Entity ID = " + gameObject.entity_id);
            return gameObject;
        }
    }
}
