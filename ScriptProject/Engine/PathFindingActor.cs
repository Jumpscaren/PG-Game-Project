using ScriptProject.EngineMath;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class PathFindingActor : Component
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern void InitComponent(UInt32 scene_index, UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern bool HasComponent(UInt32 scene_index, UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public override extern void RemoveComponent(UInt32 scene_index, UInt32 entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern Vector2 PathFind_Extern(UInt32 actor_scene_index, UInt32 actor_entity, UInt32 goal_scene_index, UInt32 goal_entity, UInt32 position_of_node_index);

        public Vector2 PathFind(GameObject goal_game_object, UInt32 position_of_node_index = 1)
        {
            return PathFind_Extern(game_object.GetSceneIndex(), game_object.GetEntityID(), goal_game_object.GetSceneIndex(), goal_game_object.GetEntityID(), position_of_node_index);
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void DebugPath();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern bool NeedNewPathFind_Extern(UInt32 actor_scene_index, UInt32 actor_entity, UInt32 goal_scene_index, UInt32 goal_entity, UInt32 position_of_node_index);

        public bool NeedNewPathFind(GameObject goal_game_object, UInt32 position_of_node_index = 1)
        {
            return NeedNewPathFind_Extern(game_object.GetSceneIndex(), game_object.GetEntityID(), goal_game_object.GetSceneIndex(), goal_game_object.GetEntityID(), position_of_node_index);
        }
    }
}
