using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.EngineMath
{
    internal class Vector2
    {
        public float x = 0.0f, y = 0.0f;

        public Vector2()
        {

        }

        public Vector2(float x, float y)
        {
            this.x = x;
            this.y = y;
        }

        public float Length()
        {
            return (float)System.Math.Sqrt(x * x + y * y);
        }

        public Vector2 Normalize()
        {
            float length = Length();
            if (length < 1e-05)
                length = 1.0f;
            return new Vector2(x, y) / length;
        }

        //https://stackoverflow.com/questions/7785601/detecting-if-angle-is-more-than-180-degrees
        static public float Angle(Vector2 v1, Vector2 v2)
        {
            float dot_product = v1.x * v2.x + v1.y * v2.y;
            float angle = (float)Math.Acos(dot_product / (v1.Length() * v2.Length()));

            float z_cross_product = v1.x * v2.y - v1.y * v2.x;
            if (z_cross_product > 0)
                return -angle;
            return angle;
        }

        public override string ToString()
        {
            return base.ToString() + ": (" + x + ", " + y + ")";
        }

        public static Vector2 operator +(Vector2 v1, Vector2 v2)
        {
            return new Vector2(v1.x + v2.x, v1.y + v2.y);
        }

        public static Vector2 operator -(Vector2 v1, Vector2 v2)
        {
            return new Vector2(v1.x - v2.x, v1.y - v2.y);
        }

        public static Vector2 operator *(Vector2 v, float scalar)
        {
            return new Vector2(v.x * scalar, v.y * scalar);
        }

        public static Vector2 operator /(Vector2 v, float scalar)
        {
            return new Vector2(v.x / scalar, v.y / scalar);
        }
    }
}
