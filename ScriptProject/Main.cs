using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject
{
    public class Main
    {
        public static int main()
        {
            System.Console.WriteLine("Hello from C# world\n");
            return 0;
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern int PrintText();

        public void TestFunc()
        {
            PrintText();
        }
    }
}
