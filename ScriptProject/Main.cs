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
        public int num = 3;
        Main i = null;
        static double health = 0.3123f;

        public Main()
        {
            i = this;
        }

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
            ReturnInt(1.0f, 0.2, "Hello", this);
            CallReturnInt(1.0f, 0.2, "Hello", this);
        }

        public void PrintArgs(int num, bool boolean, string text)
        {
            System.Console.WriteLine(text + " : " + num + " : " + boolean);
        }

        public int ReturnInt(float num1, double num2, string text, Main me)
        {
            Console.WriteLine("Num1 = " + num1);
            Console.WriteLine("Num2 = " + num2);
            Console.WriteLine("Text = " + text);
            Console.WriteLine("Me.Num = " + me.num);

            return (int)num1 + (int)num2 + text.Length + me.num; 
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern int CallReturnInt(float num1, double num2, string text, Main me);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern int Testing(int h);
    }
}
