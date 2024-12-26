using ScriptProject.Engine.Constants;
using System;

namespace ScriptProject.UserDefined
{
    internal class UserCollisionCategories
    {
        public const UInt16 PrincessCharacter = PhysicCollisionCategory.Category4;
        public const UInt16 MovingCharacter = PhysicCollisionCategory.Category2;
        public const UInt16 PrincessBlocker = PhysicCollisionCategory.Category3;

        public const UInt16 AllExceptMovingCharacter = PhysicCollisionCategory.AllCategories & ~MovingCharacter & ~PrincessCharacter;
        public const UInt16 FilterForPrincess = PhysicCollisionCategory.AllCategories & ~MovingCharacter;
        public const UInt16 FilterForPrincessBlockers = PhysicCollisionCategory.AllCategories & ~PrincessBlocker;
        public const UInt16 FilterForPrincessBlockersAndCharacters = AllExceptMovingCharacter & ~PrincessBlocker;
    }
}
