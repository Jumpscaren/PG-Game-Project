using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine.Constants
{
    internal class PhysicCollisionCategory
    {
        public const UInt16 Category1 = 0x0001; //0000-0000-0000-0001
        public const UInt16 Category2 = 0x0002; //0000-0000-0000-0010
        public const UInt16 Category3 = 0x0004;
        public const UInt16 Category4 = 0x0008;
        public const UInt16 Category5 = 0x0010;
        public const UInt16 Category6 = 0x0020;
        public const UInt16 Category7 = 0x0040;
        public const UInt16 Category8 = 0x0080;
        public const UInt16 Category9 = 0x0100;
        public const UInt16 Category10 = 0x0200;
        public const UInt16 Category11 = 0x0400;
        public const UInt16 Category12 = 0x0800;
        public const UInt16 Category13 = 0x1000;
        public const UInt16 Category14 = 0x2000;
        public const UInt16 Category15 = 0x4000;
        public const UInt16 Category16 = 0x8000; //1000-0000-0000-0000
        public const UInt16 AllCategories = 0xFFFF; //1111-1111-1111-1111
    }
}
