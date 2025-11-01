using System;

namespace Titan
{
    public struct Vector3
    {
        public float X, Y, Z;

        public static Vector3 Zero => new Vector3(0.0f);
        public static Vector3 One => new Vector3(1.0f);
        public static Vector3 Up => new Vector3(0.0f, 1.0f, 0.0f);
        public static Vector3 Down => new Vector3(0.0f, -1.0f, 0.0f);
        public static Vector3 Left => new Vector3(-1.0f, 0.0f, 0.0f);
        public static Vector3 Right => new Vector3(1.0f, 0.0f, 0.0f);
        public static Vector3 Forward => new Vector3(0.0f, 0.0f, 1.0f);
        public static Vector3 Backward => new Vector3(0.0f, 0.0f, -1.0f);

        public Vector3(float scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
        }

        public Vector3(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public Vector3(Vector2 xy, float z)
        {
            X = xy.X;
            Y = xy.Y;
            Z = z;
        }

        public Vector2 XY
        {
            get => new Vector2(X, Y);
            set {
                X = value.X;
                Y = value.Y;
            }
        }

        // --- Operators ---
        public static Vector3 operator +(Vector3 a, Vector3 b) => new Vector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);

        public static Vector3 operator -(Vector3 a, Vector3 b) => new Vector3(a.X - b.X, a.Y - b.Y, a.Z - b.Z);

        public static Vector3 operator -(Vector3 v) => new Vector3(-v.X, -v.Y, -v.Z);

        public static Vector3 operator*(Vector3 v, float scalar) => new Vector3(v.X * scalar, v.Y* scalar, v.Z* scalar);

        public static Vector3 operator*(float scalar, Vector3 v) => v * scalar;

        public static Vector3 operator /(Vector3 v, float scalar) => new Vector3(v.X / scalar, v.Y / scalar,
                                                                                 v.Z / scalar);

        public static bool operator ==(Vector3 a, Vector3 b) => a.X == b.X && a.Y == b.Y && a.Z == b.Z;

        public static bool operator !=(Vector3 a, Vector3 b) => !(a == b);

        // --- Methods ---
        public float Length() => (float)System.Math.Sqrt(X * X + Y * Y + Z * Z);

        public float LengthSquared() => X * X + Y * Y + Z * Z;

        public void Normalize()
        {
            float length = Length();
            if (length > 0)
            {
                float inv = 1.0f / length;
                X *= inv;
                Y *= inv;
                Z *= inv;
            }
        }

        public Vector3 Normalized()
        {
            float length = Length();
            return length > 0 ? this / length : Zero;
        }

        public static float Dot(Vector3 a, Vector3 b) => a.X * b.X + a.Y * b.Y + a.Z * b.Z;

        public static Vector3 Cross(Vector3 a, Vector3 b) => new Vector3(a.Y * b.Z - a.Z * b.Y, a.Z* b.X - a.X * b.Z,
                                                                         a.X* b.Y - a.Y * b.X);

        public static float Distance(Vector3 a, Vector3 b) => (a - b).Length();

        public static Vector3 Lerp(Vector3 a, Vector3 b, float t) => a + (b - a) * t;

        public override bool Equals(object obj) => obj is Vector3 v && this == v;

        public override string ToString() => $"({X:0.###}, {Y:0.###}, {Z:0.###})";
    }
}
