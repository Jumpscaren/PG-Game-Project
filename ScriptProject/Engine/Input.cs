using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.CompilerServices;

namespace ScriptProject.Engine
{
    internal class Input
    {
        public enum Key
        {
            BACKSPACE = 8, TAB,
            ENTER = 13,
            RALT = 17, LALT = 18,
            CAPSLOCK = 20,
            ESCAPE = 27,
            SPACEBAR = 32,
            LEFTARROW = 37, UPARROW, RIGHTARROW, DOWNARROW,
            NUM0 = 48, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9,
            A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
            F1 = 112, F2, F3, F4,
            LSHIFT = 160, RSHIFT, LCTRL, RCTRL,
            PERIOD = 190,
        };

        public enum MouseButton
        {
            LEFT = 0, RIGHT, WHEEL
            //Etc for more mouse buttons
        };

        public enum MouseWheelSpin
        {
            UP = 0,
		    DOWN = 1,
		    MIDDLE = 2,
	    };

        //When key is pressed
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern bool GetKeyPressed(Key key);

        //When key is down
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern bool GetKeyDown(Key key);

        //When mouse button is pressed
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern bool GetMouseButtonPressed(MouseButton key);

        //When mouse button is down
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern bool GetMouseButtonDown(MouseButton key);

        //When mouse button is down
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern bool GetMouseWheelSpin(MouseWheelSpin direction);
    }
}
