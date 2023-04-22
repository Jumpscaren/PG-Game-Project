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
        Scene scene;
        public Transform transform;

        List<Component> components = new List<Component>();

        static public GameObject CreateGameObject()
        {
            GameObject gameObject = new GameObject();
            gameObject.entity_id = SceneManager.GetActiveScene().GetEntityManager().NewEntity();
            gameObject.scene = SceneManager.GetActiveScene();
            Console.WriteLine("GameObject Entity ID = " + gameObject.entity_id);

            gameObject.transform = gameObject.AddComponent<Transform>();
            return gameObject;
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
            Console.WriteLine("GG " + scene.GetSceneIndex() + " , " + entity_id);
            component.InitComponent(scene.GetSceneIndex(), entity_id);
            components.Add(component);
            return component;
        }

        //Slow should not be used every frame
        public T GetComponent<T>() where T : Component
        {
            foreach (Component comp in components)
            {
                if (comp.GetType() == typeof(T))
                    return(T)comp;
            }

            return null;
        }
    }
}
