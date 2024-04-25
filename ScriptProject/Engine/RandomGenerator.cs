using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Engine
{
    internal class RandomGenerator
    {
        private Random m_random;

        public RandomGenerator() 
        {
            m_random = new Random();
        }

        public float RandomFloat(float min_value = 0.0f, float max_value = 1.0f)
        {
            double num = (double)max_value - (double)min_value;
            return (float)((m_random.NextDouble() * num) + (double)min_value);
        }
    }
}
