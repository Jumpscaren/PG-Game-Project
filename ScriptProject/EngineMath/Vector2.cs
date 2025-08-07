using System;

namespace ScriptProject.EngineMath
{
    internal class Vector2
    {
        public static readonly Vector2 Zero = new Vector2(0.0f, 0.0f);

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
            return new Vector2(x / length, y / length);
        }

        //https://stackoverflow.com/questions/7785601/detecting-if-angle-is-more-than-180-degrees
        static public float Angle(Vector2 v1, Vector2 v2)
        {
            float v1_length = v1.Length();
            float v2_length = v2.Length();

            if (v1_length < 1e-05)
            {
                v1_length = 1.0f;
            }
            if (v2_length < 1e-05)
            {
                v2_length = 1.0f;
            }

            float dot_product = v1.x * v2.x + v1.y * v2.y;
            float angle = (float)Math.Acos(dot_product / (v1_length * v2_length));

            float z_cross_product = v1.x * v2.y - v1.y * v2.x;
            if (z_cross_product > 0)
                return -angle;
            return angle;
        }

        public Vector2 PerpendicularClockwise()
        {
            return new Vector2(y, -x);
        }

        public Vector2 PerpendicularCounterClockwise()
        {
            return new Vector2(-y, x);
        }

        static public float ProjectVectorOnVector(Vector2 vector_a, Vector2 vector_b)
        {
            float vector_b_length = vector_b.Length();

            if (vector_b_length < 1e-05)
            {
                vector_b_length = 1.0f;
            }

            float dot_product = DotProduct(vector_a, vector_b);

            return dot_product / vector_b_length;
        }

        static public float DotProduct(Vector2 v1, Vector2 v2)
        {
            return v1.x * v2.x + v1.y * v2.y;
        }

        static public Vector2 Lerp(Vector2 v1, Vector2 v2, float t)
        {
            return v1 + (v2 - v1) * t;
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
