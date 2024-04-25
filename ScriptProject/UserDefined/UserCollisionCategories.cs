using ScriptProject.Engine.Constants;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.UserDefined
{
    internal class UserCollisionCategories
    {
        public const UInt16 MovingCharacter = PhysicCollisionCategory.Category2;
        public const UInt16 AllExceptMovingCharacter = PhysicCollisionCategory.AllCategories & ~PhysicCollisionCategory.Category2;
    }
}
