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
        private static extern void PathFind_Extern(UInt32 actor_scene_index, UInt32 actor_entity, UInt32 goal_scene_index, UInt32 goal_entity, UInt32 position_of_node_index);

        public void PathFind(GameObject goal_game_object, UInt32 position_of_node_index = 1)
        {
            PathFind_Extern(game_object.GetSceneIndex(), game_object.GetEntityID(), goal_game_object.GetSceneIndex(), goal_game_object.GetEntityID(), position_of_node_index);
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern Vector2 GetCurrentNodePosition_Extern(UInt32 actor_scene_index, UInt32 actor_entity);
        public Vector2 GetCurrentNodePosition()
        {
            return GetCurrentNodePosition_Extern(game_object.GetSceneIndex(), game_object.GetEntityID());
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern Vector2 GetNextNodePosition_Extern(UInt32 actor_scene_index, UInt32 actor_entity, UInt32 position_of_node_index);
        public Vector2 GetNextNodePosition(UInt32 position_of_node_index = 1)
        {
            return GetNextNodePosition_Extern(game_object.GetSceneIndex(), game_object.GetEntityID(), position_of_node_index);
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void ClearPath_Extern(UInt32 actor_scene_index, UInt32 actor_entity);
        public void ClearPath()
        {
            ClearPath_Extern(game_object.GetSceneIndex(), game_object.GetEntityID());
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetShowPath(bool show_path);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern bool IsPositionInWorld(Vector2 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern void GetRandomNodes(GameObject game_object, ListSetGameObject nodes, UInt32 number_of_nodes);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern GameObject GetGameObjectNodeByPosition(Vector2 position);
    }
}
