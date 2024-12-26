using ScriptProject.Engine;
using ScriptProject.EngineMath;
using ScriptProject.Scripts;
using ScriptProject.UserDefined;
using System.Runtime.CompilerServices;

namespace ScriptProject
{
    public class M
    { public int x; public int y; public int z; }

    public class Main
    {
        public int num = 3;
        Main i = null;

        public Main()
        {
            i = this;
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void cpptest();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void cpptestint(int i);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void cpptestmany(M f);

        static int loop = 10000;

        public static void test1()
        {
            for (int i = 0; i < loop; ++i)
            {
                cpptest();
            }
        }

        public static void test2()
        {
            float f = 0;
            for (int i = 0; i < loop; ++i)
            {
                f += i;
            }
        }

        public static void test3()
        {
            for (int i = 0; i < loop; ++i)
            {
                cpptestint(i);
            }
        }

        public static void test4()
        {
            M f = new M();
            for (int i = 0; i < loop; ++i)
            {
                cpptestmany(f);
            }
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

            GameObject gameMaster = GameObject.CreateGameObjectWithScene(SceneManager.GetGlobalScene());
            gameMaster.AddComponent<GameMaster>();

            PrefabSystem.CreateUserPrefab("Prefab1", Prefab1, 1, "Block");
            PrefabSystem.CreateUserPrefab("Prefab2", Prefab2, 2, "Path");
            PrefabSystem.CreateUserPrefab("WallCollider", WallCollider, 2, "Block");
            PrefabSystem.CreateUserPrefab("PlayerPrefab", PlayerPrefab, 1, "Character");
            PrefabSystem.CreateUserPrefab("PlayerCameraPrefab", PlayerCameraPrefab, 1, "Misc");
            PrefabSystem.CreateUserPrefab("BouncePrefab", BouncePrefab, 1, "Misc");
            PrefabSystem.CreateUserPrefab("BasicEnemy", BasicEnemy, 1, "Character");
            PrefabSystem.CreateUserPrefab("OrcEnemy", OrcEnemy, 1, "Character");
            PrefabSystem.CreateUserPrefab("Princess", Princess, 1, "Character");
            PrefabSystem.CreateUserPrefab("EmptyCollider", EmptyCollider, 0, "Block");
            PrefabSystem.CreateUserPrefab("Hole", Hole, 0, "Block");
            PrefabSystem.CreateUserPrefab("OrcCarrier", OrcCarrier, 1, "Character");
            PrefabSystem.CreateUserPrefab("PrincessBlocker", PrincessBlocker, 0, "Block");
            PrefabSystem.CreateUserPrefab("Finish", Finish, 0, "Block");
            PrefabSystem.CreateUserPrefab("ReplaceBlock", ReplaceBlock, 1, "Block");
            PrefabSystem.CreateUserPrefab("Switch", Switch, 1, "Interactive");
            PrefabSystem.CreateUserPrefab("OrcDistracter", OrcDistracter, 1, "Character");
            PrefabSystem.CreateUserPrefab("Fireball", Fireball, 0, "Misc");
            PrefabSystem.CreateUserPrefab("HolePolygon", HolePolygon, 0, "Block");

            return 0;
        }

        static void Prefab1(GameObject game_object)
        {
            Render.LoadTexture("../QRGameEngine/Textures/Temp_2.png", game_object.GetComponent<Sprite>());
            //game_object.AddComponent<DynamicBody>();
            game_object.AddComponent<PureStaticBody>();
            game_object.AddComponent<BoxCollider>();
        }

        static void Prefab2(GameObject game_object)
        {
            Render.LoadTexture("../QRGameEngine/Textures/Temp.png", game_object.GetComponent<Sprite>());
            game_object.AddComponent<PathFindingWorld>();
            //game_object.AddComponent<BoxCollider>();
        }

        static void WallCollider(GameObject game_object)
        {
            Render.LoadTexture("../QRGameEngine/Textures/Temp.png", game_object.GetComponent<Sprite>());
            game_object.AddComponent<PureStaticBody>();
            game_object.AddComponent<BoxCollider>();
        }

        static void PlayerPrefab(GameObject game_object)
        {
            game_object.SetName("Player");
            Render.LoadTexture("../QRGameEngine/Textures/Knight_Run_Atlas.png", game_object.GetComponent<Sprite>());
            game_object.AddComponent<AnimatableSprite>();
            game_object.AddComponent<Player>();
            game_object.AddComponent<DynamicBody>().SetFixedRotation(true);
            var circle_collider = game_object.AddComponent<CircleCollider>();
            circle_collider.SetColliderFilter(UserCollisionCategories.MovingCharacter, UserCollisionCategories.AllExceptMovingCharacter, 0);
            circle_collider.SetRadius(0.49f);
        }

        static void PlayerCameraPrefab(GameObject game_object)
        {
            Render.LoadTexture("../QRGameEngine/Textures/Knight_Temp.png", game_object.GetComponent<Sprite>());
            game_object.AddComponent<Camera>();
            game_object.AddComponent<PlayerCamera>();
            game_object.SetName("PlayerCamera");
        }

        static void Princess(GameObject game_object)
        {
            game_object.SetName("Princess");
            Render.LoadTexture("../QRGameEngine/Textures/Princess.png", game_object.GetComponent<Sprite>());
            game_object.AddComponent<DynamicBody>().SetFixedRotation(true);
            game_object.AddComponent<CircleCollider>().SetColliderFilter(UserCollisionCategories.PrincessCharacter, UserCollisionCategories.FilterForPrincess, 0);
            game_object.AddComponent<Princess>();
        }

        static void BouncePrefab(GameObject game_object)
        {
            Render.LoadTexture("../QRGameEngine/Textures/Knight_Temp.png", game_object.GetComponent<Sprite>());
            game_object.AddComponent<StaticBody>();
            game_object.AddComponent<BoxCollider>().SetTrigger(true);
            game_object.SetName("Bouncer");
        }

        static void BasicEnemy(GameObject game_object)
        {
            Render.LoadTexture("../QRGameEngine/Textures/Temp_2.png", game_object.GetComponent<Sprite>());
            game_object.AddComponent<BasicEnemy>();
            game_object.AddComponent<PathFindingActor>();
            game_object.AddComponent<DynamicBody>().SetFixedRotation(true);
            game_object.AddComponent<CircleCollider>().SetColliderFilter(UserCollisionCategories.MovingCharacter, UserCollisionCategories.AllExceptMovingCharacter, 0);
        }

        static void OrcEnemy(GameObject game_object)
        {
            Render.LoadTexture("../QRGameEngine/Textures/Orc.png", game_object.GetComponent<Sprite>());
            game_object.AddComponent<PathFindingActor>();
            game_object.AddComponent<DynamicBody>().SetFixedRotation(true);
            var collider = game_object.AddComponent<CircleCollider>();
            collider.SetColliderFilter(UserCollisionCategories.MovingCharacter, UserCollisionCategories.AllExceptMovingCharacter, 0);
            collider.SetRadius(0.49f);
            game_object.AddComponent<OrcEnemy>();
        }

        static void EmptyCollider(GameObject game_object)
        {
            game_object.AddComponent<PureStaticBody>();
            game_object.AddComponent<BoxCollider>();
            game_object.GetComponent<Sprite>();
            game_object.RemoveComponent<Sprite>();
            //game_object.AddComponent<CircleCollider>().SetColliderFilter(UserCollisionCategories.Holes, UserCollisionCategories.AllExceptMovingCharacter, 0);
        }

        static void Hole(GameObject game_object)
        {
            game_object.AddComponent<PureStaticBody>();
            game_object.SetTag(UserTags.Hole);

            var box_collider = game_object.AddComponent<BoxCollider>();
            box_collider.SetTrigger(true);
            box_collider.SetHalfBoxSize(new Vector2(0.8f, 0.8f));

            game_object.GetComponent<Sprite>();
            game_object.RemoveComponent<Sprite>();
        }

        static void PrincessBlocker(GameObject game_object)
        {
            game_object.AddComponent<PureStaticBody>();
            game_object.GetComponent<Sprite>();
            game_object.RemoveComponent<Sprite>();
            game_object.AddComponent<CircleCollider>().SetColliderFilter(UserCollisionCategories.PrincessBlocker, UserCollisionCategories.FilterForPrincess, 0);
        }

        static void OrcCarrier(GameObject game_object)
        {
            Render.LoadTexture("../QRGameEngine/Textures/OrcCarrier.png", game_object.GetComponent<Sprite>());
            game_object.AddComponent<PathFindingActor>();
            game_object.AddComponent<DynamicBody>().SetFixedRotation(true);
            var collider = game_object.AddComponent<CircleCollider>();
            collider.SetColliderFilter(UserCollisionCategories.MovingCharacter, UserCollisionCategories.AllExceptMovingCharacter, 0);
            collider.SetRadius(0.49f);
            game_object.AddComponent<OrcCarrier>();
        }

        static void Finish(GameObject game_object)
        {
            game_object.AddComponent<PureStaticBody>();
            game_object.RemoveComponent<Sprite>();
            game_object.AddComponent<BoxCollider>().SetTrigger(true);
        }

        static void ReplaceBlock(GameObject game_object)
        {
            game_object.AddComponent<PureStaticBody>();
            game_object.RemoveComponent<Sprite>();
            game_object.AddComponent<BoxCollider>();
            game_object.AddComponent<ReplaceBlockScript>();
        }

        static void Switch(GameObject game_object)
        {
            Render.LoadTexture("../QRGameEngine/Textures/UglySwitchOff.png", game_object.GetComponent<Sprite>());
            game_object.AddComponent<SwitchScript>();
        }

        static void OrcDistracter(GameObject game_object)
        {
            Render.LoadTexture("../QRGameEngine/Textures/OrcDistracter.png", game_object.GetComponent<Sprite>());
            game_object.AddComponent<PathFindingActor>();
            game_object.AddComponent<DynamicBody>().SetFixedRotation(true);
            var collider = game_object.AddComponent<CircleCollider>();
            collider.SetColliderFilter(UserCollisionCategories.MovingCharacter, UserCollisionCategories.AllExceptMovingCharacter, 0);
            collider.SetRadius(0.49f);
            game_object.AddComponent<OrcDistracter>();
        }

        static void Fireball(GameObject game_object)
        {
            Render.LoadTexture("../QRGameEngine/Textures/Fireball.png", game_object.GetComponent<Sprite>());
            game_object.AddComponent<DynamicBody>();
            game_object.AddComponent<CircleCollider>().SetTrigger(true);
            game_object.AddComponent<ProjectileScript>();
            game_object.SetName("Fireball");
        }

        static void HolePolygon(GameObject game_object)
        {
            game_object.AddComponent<PureStaticBody>();
            game_object.SetTag(UserTags.Hole);

            var polygon_collider = game_object.AddComponent<PolygonCollider>();
            polygon_collider.SetTrigger(true);

            game_object.GetComponent<Sprite>();
            game_object.RemoveComponent<Sprite>();
        }
    }
}
