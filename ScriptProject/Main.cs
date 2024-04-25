using ScriptProject.Engine;
using ScriptProject.Engine.Constants;
using ScriptProject.Scripts;
using ScriptProject.UserDefined;
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

            PrefabSystem.CreateUserPrefab("Prefab1", Prefab1, 1);
            PrefabSystem.CreateUserPrefab("Prefab2", Prefab2, 2);
            PrefabSystem.CreateUserPrefab("WallCollider", WallCollider, 2);
            PrefabSystem.CreateUserPrefab("PlayerPrefab", PlayerPrefab, 1);
            PrefabSystem.CreateUserPrefab("PlayerCameraPrefab", PlayerCameraPrefab, 1);
            PrefabSystem.CreateUserPrefab("BouncePrefab", BouncePrefab, 1);
            PrefabSystem.CreateUserPrefab("BasicEnemy", BasicEnemy, 1);
            PrefabSystem.CreateUserPrefab("OrcEnemy", OrcEnemy, 1);
            PrefabSystem.CreateUserPrefab("Princess", Princess, 1);

            return 0;
        }

        static void Prefab1(GameObject game_object)
        {
            game_object.GetComponent<Sprite>().SetTexture(Render.LoadTexture("../QRGameEngine/Textures/Temp_2.png"));
            game_object.AddComponent<DynamicBody>();
            game_object.AddComponent<BoxCollider>();
        }

        static void Prefab2(GameObject game_object)
        {
            game_object.GetComponent<Sprite>().SetTexture(Render.LoadTexture("../QRGameEngine/Textures/Temp.png"));
            game_object.AddComponent<PathFindingWorld>();
            //game_object.AddComponent<BoxCollider>();
        }

        static void WallCollider(GameObject game_object)
        {
            game_object.GetComponent<Sprite>().SetTexture(Render.LoadTexture("../QRGameEngine/Textures/Temp.png"));
            game_object.AddComponent<BoxCollider>();
        }

        static void PlayerPrefab(GameObject game_object)
        {
            game_object.SetName("Player");
            game_object.GetComponent<Sprite>().SetTexture(Render.LoadTexture("../QRGameEngine/Textures/Knight_Run_Atlas.png"));
            game_object.AddComponent<AnimatableSprite>();
            game_object.AddComponent<Player>();
            game_object.AddComponent<DynamicBody>().SetFixedRotation(true);
            game_object.AddComponent<CircleCollider>().SetColliderFilter(UserCollisionCategories.MovingCharacter, UserCollisionCategories.AllExceptMovingCharacter, 0);
        }

        static void PlayerCameraPrefab(GameObject game_object)
        {
            game_object.GetComponent<Sprite>().SetTexture(Render.LoadTexture("../QRGameEngine/Textures/Knight_Temp.png"));
            game_object.AddComponent<Camera>();
            game_object.AddComponent<PlayerCamera>();
            game_object.SetName("PlayerCamera");
        }

        static void Princess(GameObject game_object)
        {
            game_object.SetName("Princess");
            game_object.GetComponent<Sprite>().SetTexture(Render.LoadTexture("../QRGameEngine/Textures/Princess.png"));
            game_object.AddComponent<DynamicBody>().SetFixedRotation(true);
            game_object.AddComponent<CircleCollider>().SetColliderFilter(UserCollisionCategories.MovingCharacter, UserCollisionCategories.AllExceptMovingCharacter, 0);
            game_object.AddComponent<Princess>();
        }

        static void BouncePrefab(GameObject game_object)
        {
            game_object.GetComponent<Sprite>().SetTexture(Render.LoadTexture("../QRGameEngine/Textures/Knight_Temp.png"));
            game_object.AddComponent<StaticBody>();
            game_object.AddComponent<BoxCollider>().SetTrigger(true);
            game_object.SetName("Bouncer");
        }

        static void BasicEnemy(GameObject game_object)
        {
            game_object.GetComponent<Sprite>().SetTexture(Render.LoadTexture("../QRGameEngine/Textures/Temp_2.png"));
            game_object.AddComponent<BasicEnemy>();
            game_object.AddComponent<PathFindingActor>();
            game_object.AddComponent<DynamicBody>().SetFixedRotation(true);
            game_object.AddComponent<CircleCollider>().SetColliderFilter(UserCollisionCategories.MovingCharacter, UserCollisionCategories.AllExceptMovingCharacter, 0);
        }

        static void OrcEnemy(GameObject game_object)
        {
            game_object.GetComponent<Sprite>().SetTexture(Render.LoadTexture("../QRGameEngine/Textures/Orc.png"));
            game_object.AddComponent<PathFindingActor>();
            game_object.AddComponent<DynamicBody>().SetFixedRotation(true);
            game_object.AddComponent<CircleCollider>().SetColliderFilter(UserCollisionCategories.MovingCharacter, UserCollisionCategories.AllExceptMovingCharacter, 0);
            game_object.AddComponent<OrcEnemy>();
        }
    }
}
