// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;

// Define a few types that normally exist in the UnityEngine namespace so our test code can leverage
// the PlaneFinding.cs script file from the Unity addon.  This ensures that our tests are calling into
// the PlaneFinding.dll in the same way that Unity apps will be.
namespace UnityEngine
{
    public struct Vector3
    {
        public float x;
        public float y;
        public float z;

        public Vector3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public static Vector3 operator+(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
        }

        public float Length()
        {
            return (float)Math.Sqrt(x * x + y * y + z * z);
        }

        public override string ToString()
        {
            return string.Format("{{{0}, {1}, {2}}}", x, y, z);
        }

        public string ToString(string format)
        {
            return string.Format("{{{0}, {1}, {2}}}", x.ToString(format), y.ToString(format), z.ToString(format));
        }

        public static readonly Vector3 zero = new Vector3(0, 0, 0);
        public static readonly Vector3 one = new Vector3(1, 1, 1);
    }

    public struct Quaternion
    {
        public float x;
        public float y;
        public float z;
        public float w;

        public Quaternion(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public static readonly Quaternion identity = new Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    }

    public struct Matrix4x4
    {
        public float m00; public float m10; public float m20; public float m30;
        public float m01; public float m11; public float m21; public float m31;
        public float m02; public float m12; public float m22; public float m32;
        public float m03; public float m13; public float m23; public float m33;

        public Matrix4x4(
            float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23,
            float m30, float m31, float m32, float m33)
        {
            this.m00 = m00; this.m01 = m01; this.m02 = m02; this.m03 = m03;
            this.m10 = m10; this.m11 = m11; this.m12 = m12; this.m13 = m13;
            this.m20 = m20; this.m21 = m21; this.m22 = m22; this.m23 = m23;
            this.m30 = m30; this.m31 = m31; this.m32 = m32; this.m33 = m33;
        }

        public Vector3 TransformDirection(Vector3 v)
        {
            return new Vector3(
                v.x * m00 + v.y * m01 + v.z * m02,
                v.x * m10 + v.y * m11 + v.z * m12,
                v.x * m20 + v.y * m21 + v.z * m22);
        }

        public Vector3 TransformPoint(Vector3 v)
        {
            return TransformDirection(v) + new Vector3(m03, m13, m23);
        }

        public static readonly Matrix4x4 identity = new Matrix4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    }

    public struct Plane
    {
        public Vector3 normal;
        public float distance;
    };

    public struct Transform
    {
        public Matrix4x4 localToWorldMatrix;
        public Quaternion rotation;
        public Vector3 TransformPoint(Vector3 point)
        {
            return localToWorldMatrix.TransformPoint(point);
        }
    }

    public struct Bounds
    {
        public Vector3 center;
        public Vector3 extents;
    }

    public struct Mesh
    {
        public Bounds bounds;
        public Vector3[] vertices;
        public Vector3[] normals;
        public int[] triangles;
    }

    public struct MeshFilter
    {
        public Transform transform;
        public Mesh sharedMesh;
    }
}