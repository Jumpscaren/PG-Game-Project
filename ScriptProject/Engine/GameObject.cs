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

        static public GameObject CreateGameObject()
        {
            GameObject gameObject = new GameObject();
            gameObject.entity_id = SceneManager.GetActiveScene().GetEntityManager().NewEntity();
            gameObject.scene = SceneManager.GetActiveScene();
            Console.WriteLine("GameObject Entity ID = " + gameObject.entity_id);
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
            return component;
        }
    }
}
