﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class Component
    {
        //Not cool
        private bool initated = false;

        protected GameObject game_object;

        public void SetGameObject(GameObject n_game_object)
        {
            if (initated) 
            {
                return;
            }

            game_object = n_game_object;
            initated = true;
        }

        public GameObject GetGameOjbect()
        {
            return game_object;
        }

        public virtual void InitComponent(UInt32 scene_index, UInt32 entity) 
        {
            Console.WriteLine("Component is incorrectly initalized");
        }

        public virtual bool HasComponent(UInt32 scene_index, UInt32 entity)
        {
            Console.WriteLine("Component is incorrectly implemented");
            return false;
        }

        public virtual void RemoveComponent(UInt32 scene_index, UInt32 entity)
        {
            Console.WriteLine("Component is incorrectly removed");
        }
    }
}
