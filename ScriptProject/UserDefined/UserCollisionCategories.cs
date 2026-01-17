using ScriptProject.Engine.Constants;
using System;

namespace ScriptProject.UserDefined
{
    internal class UserCollisionCategories
    {
        /*
         * NOTE!!!!
         * If you change collision filter for existing prefab in a scene, make sure it has the correct filter! It will load the one from the save not the one from the script!
         */

        public const UInt16 PrincessCharacter = PhysicCollisionCategory.Category4;
        public const UInt16 MovingCharacter = PhysicCollisionCategory.Category2;
        public const UInt16 PrincessBlocker = PhysicCollisionCategory.Category3;
        public const UInt16 Shield = PhysicCollisionCategory.Category5;
        public const UInt16 MovingCharacterAvoidShield = PhysicCollisionCategory.Category6;

        public const UInt16 AllExceptMovingCharacter = PhysicCollisionCategory.AllCategories & ~MovingCharacter & ~PrincessCharacter & ~MovingCharacterAvoidShield;
        public const UInt16 FilterForPrincess = PhysicCollisionCategory.AllCategories & ~MovingCharacter & ~MovingCharacterAvoidShield;
        public const UInt16 FilterForPrincessBlockers = PhysicCollisionCategory.AllCategories & ~PrincessBlocker;
        public const UInt16 FilterForPrincessBlockersAndCharacters = AllExceptMovingCharacter & ~PrincessBlocker;

        public const UInt16 FilterForShield = AllExceptMovingCharacter & ~Shield;
        public const UInt16 FilterForAvoidShield = PhysicCollisionCategory.AllCategories & ~MovingCharacterAvoidShield;

        public const UInt16 AllCategories = PhysicCollisionCategory.AllCategories;
    }
}
