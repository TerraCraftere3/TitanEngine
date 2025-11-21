using System;

namespace Titan
{
    public struct Vector4
    {
        public float X, Y, Z, W;

        public static Vector4 Zero => new Vector4(0.0f);
        public static Vector4 One => new Vector4(1.0f);
        public static Vector4 UnitX => new Vector4(1.0f, 0.0f, 0.0f, 0.0f);
        public static Vector4 UnitY => new Vector4(0.0f, 1.0f, 0.0f, 0.0f);
        public static Vector4 UnitZ => new Vector4(0.0f, 0.0f, 1.0f, 0.0f);
        public static Vector4 UnitW => new Vector4(0.0f, 0.0f, 0.0f, 1.0f);

        public Vector4(float scalar) { X = Y = Z = W = scalar; }

        public Vector4(float x, float y, float z, float w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        public Vector4(Vector3 xyz, float w)
        {
            X = xyz.X;
            Y = xyz.Y;
            Z = xyz.Z;
            W = w;
        }

        public Vector3 XYZ
        {
            get => new Vector3(X, Y, Z);
            set {
                X = value.X;
                Y = value.Y;
                Z = value.Z;
            }
        }

        public Vector4 WithW(float w) => new Vector4(X, Y, Z, w);

        // --- Operators ---
        public static Vector4 operator +(Vector4 a, Vector4 b) => new Vector4(a.X + b.X, a.Y + b.Y, a.Z + b.Z,
                                                                              a.W + b.W);

        public static Vector4 operator -(Vector4 a, Vector4 b) => new Vector4(a.X - b.X, a.Y - b.Y, a.Z - b.Z,
                                                                              a.W - b.W);

        public static Vector4 operator -(Vector4 v) => new Vector4(-v.X, -v.Y, -v.Z, -v.W);

        public static Vector4 operator*(Vector4 v, float scalar) => new Vector4(v.X * scalar, v.Y* scalar, v.Z* scalar,
                                                                                v.W* scalar);

        public static Vector4 operator*(float scalar, Vector4 v) => v * scalar;

        public static Vector4 operator /(Vector4 v, float scalar) => new Vector4(v.X / scalar, v.Y / scalar,
                                                                                 v.Z / scalar, v.W / scalar);

        public static bool operator ==(Vector4 a, Vector4 b) => a.X == b.X && a.Y == b.Y && a.Z == b.Z && a.W == b.W;

        public static bool operator !=(Vector4 a, Vector4 b) => !(a == b);

        public override int GetHashCode()
        {
            unchecked
            {
                int hash = 17;
                hash = hash * 23 + X.GetHashCode();
                hash = hash * 23 + Y.GetHashCode();
                hash = hash * 23 + Z.GetHashCode();
                hash = hash * 23 + W.GetHashCode();
                return hash;
            }
        }

        // --- Methods ---
        public float Length() => (float)Math.Sqrt(X * X + Y * Y + Z * Z + W * W);

        public float LengthSquared() => X * X + Y * Y + Z * Z + W * W;

        public void Normalize()
        {
            float length = Length();
            if (length > 0)
            {
                float inv = 1.0f / length;
                X *= inv;
                Y *= inv;
                Z *= inv;
                W *= inv;
            }
        }

        public Vector4 Normalized()
        {
            float length = Length();
            return length > 0 ? this / length : Zero;
        }

        public static float Dot(Vector4 a, Vector4 b) => a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;

        public static float Distance(Vector4 a, Vector4 b) => (a - b).Length();

        public static Vector4 Lerp(Vector4 a, Vector4 b, float t) => a + (b - a) * t;

        public override bool Equals(object obj) => obj is Vector4 v && this == v;

        public override string ToString() => $"({X:0.###}, {Y:0.###}, {Z:0.###}, {W:0.###})";
    }
}
