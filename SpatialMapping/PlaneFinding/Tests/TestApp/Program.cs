// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

using Microsoft.MixedReality.Toolkit.SpatialAwareness.Processing;
using System;
using System.Collections.Generic;
using UnityEngine;

namespace PlaneFindingTestApp
{
    class Program
    {
        static void Main()
        {
            List<PlaneFinding.MeshData> meshes = new List<PlaneFinding.MeshData>
            {
                Util.CreateSimpleMesh(10, new Vector3(5, 5, 0))
            };

            BoundedPlane[] planes = PlaneFinding.FindPlanes(meshes, 0.0f, 0.0f);

            Console.WriteLine($"Found {planes.Length} plane{(planes.Length != 1 ? "s" : "")}");
            for (int i = 0; i < planes.Length; ++i)
            {
                Console.WriteLine("{0}:", i);
                Console.WriteLine("   Area:    {0}", planes[i].Area.ToString("0.000"));
                Console.WriteLine("   Center:  {0}", planes[i].Bounds.Center.ToString("0.000"));
                Console.WriteLine("   Extents: {0}", planes[i].Bounds.Extents.ToString("0.000"));
                Console.WriteLine("   Normal:  {0}", planes[i].Plane.normal.ToString("0.000"));
                Console.WriteLine();
            }
            Console.ReadKey();
        }
    }
}
