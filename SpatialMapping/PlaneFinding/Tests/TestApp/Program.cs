// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using UnityEngine;
using HoloToolkit.Unity;

namespace PlaneFindingTestApp
{
    class Program
    {
        static void Main(string[] args)
        {
            List<PlaneFinding.MeshData> meshes = new List<PlaneFinding.MeshData>();
            meshes.Add(Util.CreateSimpleMesh(10, new Vector3(5, 5, 0)));

            BoundedPlane[] planes = PlaneFinding.FindPlanes(meshes, 0.0f, 0.0f);

            Console.WriteLine("Found {0} meshes", planes.Length);
            for (int i = 0; i < planes.Length; ++i)
            {
                Console.WriteLine("{0}:", i);
                Console.WriteLine("   Area:    {0}", planes[i].Area.ToString("0.000"));
                Console.WriteLine("   Center:  {0}", planes[i].Bounds.Center.ToString("0.000"));
                Console.WriteLine("   Extents: {0}", planes[i].Bounds.Extents.ToString("0.000"));
                Console.WriteLine("   Normal:  {0}", planes[i].Plane.normal.ToString("0.000"));
                Console.WriteLine();
            }
        }
    }
}
