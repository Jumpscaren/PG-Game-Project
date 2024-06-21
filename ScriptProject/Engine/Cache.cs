using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class Cache<T>
    {
        private T data;
        private float timestamp;

        public bool IsDataOld()
        {
            return timestamp != Time.GetElapsedTime();
        }

        public void CacheData(T new_data)
        {
            data = new_data;
            timestamp = Time.GetElapsedTime();
        }

        public T GetData()
        {
            return data;
        }
    }
}
