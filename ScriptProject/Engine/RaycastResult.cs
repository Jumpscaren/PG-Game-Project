using ScriptProject.EngineMath;

namespace ScriptProject.Engine
{
    internal class RaycastResult
    {
#pragma warning disable 649
        public bool intersected = false;
        public GameObject intersected_game_object = null;
        public Vector2 intersected_position = null;
#pragma warning restore 649
    }
}
