using ScriptProject.Engine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.EngineFramework
{
    internal class Timer
    {
        float timer = 0.0f;
        float time_limit = 0.0f;

        public Timer() { }

        public Timer(float in_time_limit)
        {
            SetTimeLimit(in_time_limit);
        }

        public void SetTimeLimit(float in_time_limit)
        {
            time_limit = in_time_limit;
        }

        public float GetTimeLimit()
        {
            return time_limit;
        }

        public void Start()
        {
            timer = time_limit + Time.GetElapsedTime();
        }

        public bool IsExpired()
        {
            return timer < Time.GetElapsedTime();
        }

        public float GetTime()
        {
            return timer;
        }
    }
}
