using ScriptProject.Engine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace ScriptProject
{
    public class Main
    {
        public int num = 3;
        Main i = null;

        public Main()
        {
            i = this;
        }

        public static int main()
        {
            //System.Console.WriteLine("Hello from C# world\n");

            //GameObject gameObject = GameObject.CreateGameObject();
            //Sprite sprite = gameObject.AddComponent<Sprite>();
            //gameObject.GetComponent<Transform>().SetPosition(0.0f, 0.0f);
            //gameObject.GetComponent<Transform>().SetZIndex(2);

            //gameObject.AddComponent<TestScript>();

            //string texture_path = "../QRGameEngine/Textures/Temp.png";
            //Texture texture = Render.LoadTexture(texture_path);
            //sprite.SetTexture(texture);

            PrefabSystem.CreateUserPrefab(Prefab1, 1);
            PrefabSystem.CreateUserPrefab(Prefab2, 2);

            return 0;
        }

        static void Prefab1(GameObject game_object)
        {
            game_object.GetComponent<Sprite>().SetTexture(Render.LoadTexture("../QRGameEngine/Textures/Temp_2.png"));
            game_object.AddComponent<TestScript>();
            game_object.AddComponent<DynamicBody>();
            game_object.AddComponent<BoxCollider>();
        }

        static void Prefab2(GameObject game_object)
        {
            game_object.GetComponent<Sprite>().SetTexture(Render.LoadTexture("../QRGameEngine/Textures/Temp.png"));
            game_object.AddComponent<CircleCollider>();
        }
    }
}
