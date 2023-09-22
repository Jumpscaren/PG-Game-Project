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
            System.Console.WriteLine("Hello from C# world\n");

            GameObject gameObject = GameObject.CreateGameObject();
            Sprite sprite = gameObject.AddComponent<Sprite>();
            gameObject.GetComponent<Transform>().SetPosition(0.0f, 0.0f);
            gameObject.GetComponent<Transform>().SetZIndex(2);

            gameObject.AddComponent<TestScript>();

            string texture_path = "../QRGameEngine/Textures/Temp.png";
            Texture texture = Render.LoadTexture(texture_path);
            sprite.SetTexture(texture);

            PrefabSystem.CreateUserPrefab(Prefab1, 1);
            PrefabSystem.CreateUserPrefab(Prefab2, 2);

            return 0;
        }

        float x = 0;
        public void Update()
        {
            x += 1;
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern int PrintText();

        public void TestFunc()
        {
            PrintText();
            ReturnInt(1.0f, 0.2, "Hello", this);
            CallReturnInt(1.0f, 0.2, "Hello", this);
        }

        public void PrintArgs(int num, bool boolean, string text)
        {
            System.Console.WriteLine(text + " : " + num + " : " + boolean);
        }

        public int ReturnInt(float num1, double num2, string text, Main me)
        {
            Console.WriteLine("Num1 = " + num1);
            Console.WriteLine("Num2 = " + num2);
            Console.WriteLine("Text = " + text);
            Console.WriteLine("Me.Num = " + me.num);

            return (int)num1 + (int)num2 + text.Length + me.num; 
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern int CallReturnInt(float num1, double num2, string text, Main me);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern int Testing(int h);

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
