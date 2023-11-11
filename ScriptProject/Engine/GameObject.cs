using System;
using System.Collections.Generic;
using System.Linq;
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

        //List<Component> components = new List<Component>();

        public static bool operator ==(GameObject left, GameObject right)
        {
            return left.entity_id == right.entity_id && left.scene.GetSceneIndex() == right.scene.GetSceneIndex();
        }
        public static bool operator !=(GameObject left, GameObject right)
        {
            return left.entity_id != right.entity_id || left.scene.GetSceneIndex() != right.scene.GetSceneIndex();
        }

        public override bool Equals(object obj)
        {
            var other = obj as GameObject;

            if (other == null)
                return false;

            return this.entity_id == other.entity_id && this.scene.GetSceneIndex() == other.scene.GetSceneIndex();
        }

        public override int GetHashCode() 
        { 
            return this.entity_id.GetHashCode();
        }

        static private GameObject NewGameObjectWithExistingEntity(UInt32 entity, Scene scene)
        {
            GameObject gameObject = new GameObject();
            gameObject.entity_id = entity;
            gameObject.scene = scene;
            if (gameObject.HasComponent<Transform>())
                gameObject.transform = gameObject.GetComponent<Transform>();
            else
                gameObject.transform = gameObject.AddComponent<Transform>();
            gameObject.AddEntityData();

            return gameObject;
        }

        static public GameObject CreateGameObject()
        {
            GameObject gameObject = new GameObject();
            gameObject.entity_id = SceneManager.GetActiveScene().GetEntityManager().NewEntity();
            gameObject.scene = SceneManager.GetActiveScene();
            gameObject.AddEntityData();
            //Console.WriteLine("GameObject Entity ID = " + gameObject.entity_id);

            gameObject.transform = gameObject.AddComponent<Transform>();
            gameObject.transform.SetZIndex(1);
            return gameObject;
        }

        static public void DeleteGameObject()
        {

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
            //Console.WriteLine("GG " + scene.GetSceneIndex() + " , " + entity_id);
            component.InitComponent(scene.GetSceneIndex(), entity_id);
            //components.Add(component);
            return component;
        }

        //Slow should not be used every frame
        public T GetComponent<T>() where T : Component, new()
        {
            //foreach (Component comp in components)
            //{
            //    if (comp.GetType() == typeof(T))
            //        return (T)comp;
            //}

            if (HasComponent<T>())
            {
                T component = new T();
                component.SetGameObject(this);
                //components.Add(component);
                return component;
            }

            Console.WriteLine("ERROR: COULDN'T FIND COMPONENT");

            return null;
        }

        private bool HasComponent<T>() where T : Component, new()
        {
            T component = new T();
            return component.HasComponent(scene.GetSceneIndex(), entity_id);
        }

        public void RemoveComponent<T>() where T : Component, new()
        {
            T component = new T();
            component.SetGameObject(this);
            component.RemoveComponent(scene.GetSceneIndex(), entity_id);

            //foreach (Component comp in components)
            //{
            //    if (comp.GetType() == typeof(T))
            //    {
            //        comp.RemoveComponent(scene.GetSceneIndex(), entity_id);
            //        components.Remove(comp);
            //        break;
            //    }
            //}
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern void AddEntityData();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetName(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern string GetName();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern GameObject TempFindGameObject(string name);
    }
}
