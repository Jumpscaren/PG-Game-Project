using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.EngineMath
{
    internal class Vector4
    {
        public static readonly Vector4 Zero = new Vector4(0.0f, 0.0f, 0.0f, 0.0f);

        public float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

        public float r
        {
            get { return x; }
            set { x = value; }
        }

        public float g
        {
            get { return y; }
            set { y = value; }
        }

        public float b
        {
            get { return z; }
            set { z = value; }
        }

        public float a
        {
            get { return w; }
            set { w = value; }
        }

        public Vector4()
        {

        }

        public Vector4(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public float Length()
        {
            return (float)System.Math.Sqrt(DotProduct(this, this));
        }

        public Vector4 Normalize()
        {
            float length = Length();
            if (length < 1e-05)
                length = 1.0f;
            return new Vector4(x / length, y / length, z / length, w / length);
        }

        static public float DotProduct(Vector4 v1, Vector4 v2)
        {
            float x_sqr = v1.x * v2.x;
            float y_sqr = v1.y * v2.y;
            float z_sqr = v1.z * v2.z;
            float w_sqr = v1.w * v2.w;
            return x_sqr + y_sqr + z_sqr + w_sqr;
        }

        static public Vector4 Lerp(Vector4 v1, Vector4 v2, float t)
        {
            return v1 + (v2 - v1) * t;
        }

        public override string ToString()
        {
            return base.ToString() + ": (" + x + ", " + y + ", " + z + ", " + w + ")";
        }

        public static Vector4 operator +(Vector4 v1, Vector4 v2)
        {
            return new Vector4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
        }

        public static Vector4 operator -(Vector4 v1, Vector4 v2)
        {
            return new Vector4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
        }

        public static Vector4 operator *(Vector4 v, float scalar)
        {
            return new Vector4(v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar);
        }

        public static Vector4 operator /(Vector4 v, float scalar)
        {
            return new Vector4(v.x / scalar, v.y / scalar, v.z / scalar, v.w / scalar);
        }
    }
}
