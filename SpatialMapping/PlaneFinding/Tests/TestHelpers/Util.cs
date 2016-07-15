// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System.Linq;
using UnityEngine;
using HoloToolkit.Unity;

public static class Util
{
    /// <summary>
    /// Helper function that creates a simple mesh for use in the PlaneFinding API. The mesh is built as tesselated
    /// quad spanning the x and y dimensions of the specified Bounds. Each vert in the mesh is randomly perturbed
    /// along the z axis with in the specified Bounds. This results in a somewhat more realistic noisy planar mesh
    /// that simulates the sort of data we might get from SR.
    /// </summary>
    ///
    public static PlaneFinding.MeshData CreateSimpleMesh(int dimension, Vector3 bounds, Matrix4x4 vertDataTransform, Matrix4x4 meshTransform)
    {
        System.Random r = new System.Random();
        Vector3[] vertices = Enumerable.Range(0, dimension + 1)
            .SelectMany(x => Enumerable.Range(0, dimension + 1)
                .Select(y => new Vector3(
                        bounds.x * (2.0f * (float)x / (float)(dimension) - 1.0f),
                        bounds.y * (2.0f * (float)y / (float)(dimension) - 1.0f),
                        bounds.z * (2.0f * (float)r.NextDouble() - 1.0f))
                )
            )
            .Select(p => vertDataTransform.TransformPoint(p))
            .ToArray();

        Vector3[] normals = Enumerable.Repeat(new Vector3(0, 0, 1), (dimension + 1) * (dimension + 1))
            .Select(n => vertDataTransform.TransformDirection(n))
            .ToArray();

        int[] indices = Enumerable.Range(0, dimension)
            .SelectMany(x => Enumerable.Range(0, dimension)
                .SelectMany(y => new int[]
                {
                    ((dimension + 1) * (x + 0) + (y + 0)),
                    ((dimension + 1) * (x + 1) + (y + 0)),
                    ((dimension + 1) * (x + 0) + (y + 1)),
                    ((dimension + 1) * (x + 0) + (y + 1)),
                    ((dimension + 1) * (x + 1) + (y + 0)),
                    ((dimension + 1) * (x + 1) + (y + 1)),
                })
            ).ToArray();
        
        PlaneFinding.MeshData mesh = new PlaneFinding.MeshData();
        mesh.Transform = meshTransform;
        mesh.Verts = vertices;
        mesh.Normals = normals;
        mesh.Indices = indices;
        return mesh;
    }
    
    public static PlaneFinding.MeshData CreateSimpleMesh(int dimension, Vector3 bounds)
    {
        return CreateSimpleMesh(dimension, bounds, Matrix4x4.identity, Matrix4x4.identity);
    }

    public static PlaneFinding.MeshData CreateSimpleMesh(int dimension, Vector3 bounds, Matrix4x4 transform)
    {
        return CreateSimpleMesh(dimension, bounds, transform, Matrix4x4.identity);
    }
}
