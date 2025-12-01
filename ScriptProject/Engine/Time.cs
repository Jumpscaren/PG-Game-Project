using ScriptProject.Engine.Constants;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class Time
    {
        static private float m_delta_time = 0;
        static private float m_elapsed_time = 0;

        static private void SetDeltaTime(float delta_time)
        {
            m_delta_time = delta_time;
        }

        static private void SetElapsedTime(float elapsed_time)
        {
            m_elapsed_time = elapsed_time;
        }

        static public float GetDeltaTime() 
        {
            return m_delta_time;
        }

        static public float GetFixedDeltaTime()
        {
            return PhysicConstants.TIME_STEP;
        }

        static public float GetElapsedTime()
        {
            return m_elapsed_time;
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        static public extern float GetPreciseElapsedTime();
    }
}
