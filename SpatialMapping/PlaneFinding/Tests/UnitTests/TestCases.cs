// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using UnityEngine;
using HoloToolkit.Unity;

namespace PlaneFindingUnitTests
{
    [TestClass]
    public class TestCases
    {
        #region Utils

        private static void AssertAreEqual(Vector3 expected, Vector3 actual, float delta, string vectorName)
        {
            Assert.AreEqual(expected.x, actual.x, delta, "{0}.x", vectorName);
            Assert.AreEqual(expected.y, actual.y, delta, "{0}.y", vectorName);
            Assert.AreEqual(expected.z, actual.z, delta, "{0}.z", vectorName);
        }

        private static void AssertExpectedPlane(BoundedPlane plane, Vector3 bounds, Matrix4x4 vertDataTransform, Matrix4x4 meshTransform)
        {
            float expectedArea = 4.0f * bounds.x * bounds.y;
            Vector3 expectedPlaneNormal = meshTransform.TransformDirection(vertDataTransform.TransformDirection(new Vector3(0, 0, 1)));
            Vector3 expectedPlaneCenter = meshTransform.TransformPoint(vertDataTransform.TransformPoint(Vector3.zero));

            Assert.AreEqual(expectedArea, plane.Area, expectedArea * 0.01f);
            AssertAreEqual(expectedPlaneNormal, plane.Plane.normal, 0.01f, "Normal");
            AssertAreEqual(expectedPlaneCenter, plane.Bounds.Center, bounds.Length() * 0.01f, "Center");
        }

        private static void TestSimplePlane(int dimension, Vector3 bounds)
        {
            TestSimplePlane(dimension, bounds, Matrix4x4.identity, Matrix4x4.identity);
        }

        private static void TestSimplePlane(int dimension, Vector3 bounds, Matrix4x4 transform)
        {
            TestSimplePlane(dimension, bounds, transform, Matrix4x4.identity);
        }

        private static void TestSimplePlane(int dimension, Vector3 bounds, Matrix4x4 vertDataTransform, Matrix4x4 meshTransform)
        {
            List<PlaneFinding.MeshData> meshes = new List<PlaneFinding.MeshData>();
            meshes.Add(Util.CreateSimpleMesh(dimension, bounds, vertDataTransform, meshTransform));

            BoundedPlane[] planes = PlaneFinding.FindPlanes(meshes, 0.0f, 0.0f);
            Assert.AreEqual(1, planes.Length);
            AssertExpectedPlane(planes[0], bounds, vertDataTransform, meshTransform);
        }

        #endregion

        [TestMethod]
        public void TestSimplePlane_100Verts()
        {
            TestSimplePlane(10, new Vector3(5, 5, 0.01f));
        }

        [TestMethod]
        public void TestSimplePlane_10000Verts()
        {
            TestSimplePlane(100, new Vector3(5, 5, 0.01f));
        }

        [TestMethod]
        public void TestSimplePlane_TranslatedMesh()
        {
            Matrix4x4 vertDataTransform = new Matrix4x4(
               1, 0, 0, 31,
               0, 1, 0, -9,
               0, 0, 1, 52,
               0, 0, 0, 1);

            TestSimplePlane(10, new Vector3(5, 5, 0.01f), vertDataTransform);
        }

        [TestMethod]
        public void TestSimplePlane_RotatedMesh()
        {
            Matrix4x4 vertDataTransform = new Matrix4x4(
                1, 0, 0, 0,
                0, 0, 1, 0,
                0, -1, 0, 0,
                0, 0, 0, 1);

            TestSimplePlane(10, new Vector3(5, 5, 0.01f), vertDataTransform);
        }

        [TestMethod]
        public void TestSimplePlane_TranslatedRotatedMesh()
        {
            Matrix4x4 vertDataTransform = new Matrix4x4(
                0, -1, 0, 15,
                1, 0, 0, 11,
                0, 0, 1, -9,
                0, 0, 0, 1);

            TestSimplePlane(10, new Vector3(5, 5, 0.01f), vertDataTransform);
        }

        [TestMethod]
        public void TestSimplePlane_OffCenter_TranslatedRotatedMesh()
        {
            Matrix4x4 vertDataTransform = new Matrix4x4(
                1, 0, 0, 3,
                0, 1, 0, -12,
                0, 0, 1, -5,
                0, 0, 0, 1);

            Matrix4x4 meshDataTransform = new Matrix4x4(
                0, 0, -1, 7,
                0, 1, 0, 71,
                1, 0, 0, 14,
                0, 0, 0, 1);

            TestSimplePlane(10, new Vector3(5, 5, 0.01f), vertDataTransform, meshDataTransform);
        }
    }
}
