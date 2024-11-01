using ScriptProject.EngineMath;
using ScriptProject.UserDefined;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class GameObject
    {
        UInt32 entity_id;
        Scene scene;
        public Transform transform;
        bool destroyed;

        private Cache<GameObject> cache_parent = new Cache<GameObject>();

        private static Dictionary<UInt32, Dictionary<UInt32, GameObject>> game_object_database = new Dictionary<uint, Dictionary<uint, GameObject>>();
        private static Dictionary<UInt32, Dictionary<UInt32, Dictionary<String, Component>>> scene_to_component_map = new Dictionary<uint, Dictionary<uint, Dictionary<string, Component>>>();

        public static bool operator ==(GameObject left, GameObject right)
        {
            if (ReferenceEquals(left, right))
            {
                return true;
            }

            if (left is null && !(right is null))
            {
                return right.destroyed;
            }

            if (right is null && !(left is null))
            {
                return left.destroyed;
            }

            return left.entity_id == right.entity_id && left.scene.GetSceneIndex() == right.scene.GetSceneIndex();
        }
        public static bool operator !=(GameObject left, GameObject right)
        {
            return !(left == right);
        }

        public override bool Equals(object obj)
        {
            var other = obj as GameObject;

            if (ReferenceEquals(this, obj))
            {
                return true;
            }

            if (this is null && !(other is null))
            {
                return other.destroyed;
            }

            if (other is null && !(this is null))
            {
                return this.destroyed;
            }

            return this.entity_id == other.entity_id && this.scene.GetSceneIndex() == other.scene.GetSceneIndex();
        }

        public override int GetHashCode() 
        { 
            return this.entity_id.GetHashCode();
        }

        static private GameObject NewGameObjectWithExistingEntity(UInt32 entity, Scene scene)
        {
            GameObject game_object = GetGameObjectFromDatabase(scene.GetSceneIndex(), entity);

            if (game_object == null)
            {
                game_object = new GameObject();
                game_object.entity_id = entity;
                game_object.scene = scene;
                if (game_object.HasComponent<Transform>())
                    game_object.transform = game_object.GetComponent<Transform>();
                else
                    game_object.transform = game_object.AddComponent<Transform>();
                game_object.AddEntityData();

                AddGameObjectToDatabase(game_object);
            }

            return game_object;
        }

        static public GameObject CreateGameObject()
        {
            GameObject game_object = new GameObject();
            game_object.entity_id = SceneManager.GetActiveScene().GetEntityManager().NewEntity();
            game_object.scene = SceneManager.GetActiveScene();
            game_object.AddEntityData();

            game_object.transform = game_object.AddComponent<Transform>();
            game_object.transform.SetZIndex(1);

            AddGameObjectToDatabase(game_object);

            return game_object;
        }

        static public void DeleteGameObject(GameObject game_object)
        {
            SceneManager.GetActiveScene().GetEntityManager().RemoveEntity(game_object);
            game_object.DestroyChildren();
            //game_object = null;
        }

        public UInt32 GetEntityID() 
        { 
            return entity_id; 
        }

        public UInt32 GetSceneIndex()
        {
            return scene.GetSceneIndex();
        }

        public Scene GetScene()
        {
            return scene;
        }

        public T AddComponent<T>() where T : Component, new()
        {
            T component = new T();
            component.SetGameObject(this);
            component.InitComponent(scene.GetSceneIndex(), entity_id);
            //Handle the case when you remove the component in c++ world and it still exists in c# world and then try to add a component in c#
            {
                T old_component = GetComponentFromSceneComponentMap<T>(scene.GetSceneIndex(), entity_id, false);
                if (old_component != null)
                {
                    RemoveComponentFromSceneToComponentMap<T>(scene.GetSceneIndex(), entity_id);
                }
            }
            AddComponentToSceneComponentMap(scene.GetSceneIndex(), entity_id, component);
            return component;
        }

        //Slow should not be used every frame
        public T GetComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>())
            {
                T component = GetComponentFromSceneComponentMap<T>(scene.GetSceneIndex(), entity_id);
                if (component == null)
                {
                    component = new T();
                    component.SetGameObject(this);
                    AddComponentToSceneComponentMap(scene.GetSceneIndex(), entity_id, component);
                }
                return component;
            }

            Console.WriteLine("ERROR: COULDN'T FIND COMPONENT (" + typeof(T) + ")");

            return null;
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            T component = new T();
            return component.HasComponent(scene.GetSceneIndex(), entity_id);
        }

        public void RemoveComponent<T>() where T : Component, new()
        {
            T component = new T();
            component.SetGameObject(this);
            component.RemoveComponent(scene.GetSceneIndex(), entity_id);
            RemoveComponentFromSceneToComponentMap<T>(scene.GetSceneIndex(), entity_id);
        }

        private static string GetComponentName<T>()
        {
            if (typeof(ScriptingBehaviour).IsAssignableFrom(typeof(T)))
            {
                return typeof(ScriptingBehaviour).Name;
            }
            return typeof(T).Name;
        }

        static private void AddComponentToSceneComponentMap<T>(UInt32 scene_index, UInt32 entity, T component) where T : Component, new()
        {
            Dictionary<UInt32, Dictionary<String, Component>> entity_to_component;
            if (!scene_to_component_map.TryGetValue(scene_index, out entity_to_component))
            {
                entity_to_component = new Dictionary<UInt32, Dictionary<String, Component>>();
                scene_to_component_map.Add(scene_index, entity_to_component);
            }
            Dictionary<String, Component> name_to_component;
            if (!entity_to_component.TryGetValue(entity, out name_to_component))
            {
                name_to_component = new Dictionary<String, Component>();
                entity_to_component.Add(entity, name_to_component);
            }
            name_to_component.Add(GetComponentName<T>(), component);
        }

        static private T GetComponentFromSceneComponentMap<T>(UInt32 scene_index, UInt32 entity, bool warn = true) where T : Component, new()
        {
            Dictionary<UInt32, Dictionary<String, Component>> entity_to_component;
            if (!scene_to_component_map.TryGetValue(scene_index, out entity_to_component))
            {
                if (warn)
                {
                    Console.WriteLine("Scene Doesn't Exist In Scene To Component Map:" + scene_index);
                }
                return null;
            }
            Dictionary<String, Component> name_to_component;
            if (!entity_to_component.TryGetValue(entity, out name_to_component))
            {
                if (warn)
                {
                    Console.WriteLine("Entity Doesn't Exist In Entity To Component Map: " + entity);
                }
                return null;
            }
            Component return_component;
            if (!name_to_component.TryGetValue(GetComponentName<T>(), out return_component))
            {
                //Console.WriteLine("Name Doesn't Exist In Name To Component Map: " + typeof(T).Name);
                return null;
            }
            return (T)return_component;
        }

        static private void RemoveSceneFromSceneToComponentMap(UInt32 scene_index)
        {
            if (!scene_to_component_map.Remove(scene_index))
            {
                Console.WriteLine("RemoveSceneFromSceneToComponentMap: Scene Doesn't Exist In Scene To Component Map:" + scene_index);
            }
        }

        static private void RemoveEntityFromSceneToComponentMap(UInt32 scene_index, UInt32 entity)
        {
            Dictionary<UInt32, Dictionary<String, Component>> entity_to_component;
            if (!scene_to_component_map.TryGetValue(scene_index, out entity_to_component))
            {
                Console.WriteLine("RemoveEntityFromSceneToComponentMap: Scene Doesn't Exist In Scene To Component Map:" + scene_index);
            }
            if (!entity_to_component.Remove(entity))
            {
                Console.WriteLine("RemoveEntityFromSceneToComponentMap: Entity Doesn't Exist In Entity To Component Map:" + entity);
            }
        }

        static private void RemoveComponentFromSceneToComponentMap<T>(UInt32 scene_index, UInt32 entity) where T : Component, new()
        {
            Dictionary<UInt32, Dictionary<String, Component>> entity_to_component;
            if (!scene_to_component_map.TryGetValue(scene_index, out entity_to_component))
            {
                Console.WriteLine("RemoveComponentFromSceneToComponentMap: Scene Doesn't Exist In Scene To Component Map:" + scene_index);
            }
            Dictionary<String, Component> name_to_component;
            if (!entity_to_component.TryGetValue(entity, out name_to_component))
            {
                Console.WriteLine("RemoveComponentFromSceneToComponentMap: Entity Doesn't Exist In Entity To Component Map: " + entity);
            }
            if (!name_to_component.Remove(GetComponentName<T>()))
            {
                //Console.WriteLine("RemoveComponentFromSceneToComponentMap: Name Doesn't Exist In Name To Component Map: " + GetComponentName<T>());
            }
        }

        static private void AddGameObjectToDatabase(GameObject game_object)
        {
            Dictionary<UInt32, GameObject> game_objects;
            if (!game_object_database.TryGetValue(game_object.scene.GetSceneIndex(), out game_objects))
            {
                game_objects = new Dictionary<UInt32, GameObject>();
                game_object_database.Add(game_object.scene.GetSceneIndex(), game_objects);
            }
            game_objects.Add(game_object.entity_id, game_object);
        }

        static private GameObject GetGameObjectFromDatabase(UInt32 scene_index, UInt32 entity)
        {
            Dictionary<UInt32, GameObject> game_objects;
            if (game_object_database.TryGetValue(scene_index, out game_objects))
            {
                GameObject game_object;
                if (game_objects.TryGetValue(entity, out game_object))
                {
                    return game_object;
                }
            }
            return null;
        }

        static private void RemoveGameObjectFromDatabase(UInt32 scene_index, UInt32 entity)
        {
            Dictionary<UInt32, GameObject> game_objects;
            if (game_object_database.TryGetValue(scene_index, out game_objects))
            {
                if (game_objects.ContainsKey(entity))
                {
                    game_objects[entity].destroyed = true;
                    game_objects.Remove(entity);
                }
            }
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern void AddEntityData();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetName(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern string GetName();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern GameObject TempFindGameObject(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern GameObject FindGameObjectWithTag(Byte tag);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static private extern void FindGameObjectsWithTag_Extern(ListSetGameObject list, Byte tag);

        static public List<GameObject> FindGameObjectsWithTag(Byte tag)
        {
            ListSetGameObject list = new ListSetGameObject();
            FindGameObjectsWithTag_Extern(list, tag);
            return list.GetData();
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void AddChild(GameObject child_game_object);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void RemoveChild(GameObject child_game_object);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern bool HasChildren();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern bool DestroyChildren();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern GameObject GetParent_Extern(UInt32 scene_index, UInt32 entity);

        public GameObject GetParent()
        {
            if (cache_parent.IsDataOld())
            {
                cache_parent.CacheData(GetParent_Extern(scene.GetSceneIndex(), entity_id));
            }

            return cache_parent.GetData();
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern bool HasParent();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetTag(Byte tag);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern Byte GetTag();
    }
}
