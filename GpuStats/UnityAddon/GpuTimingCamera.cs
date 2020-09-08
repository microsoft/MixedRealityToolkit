// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

using UnityEngine;

namespace Microsoft.MixedReality.Toolkit.Utilities
{
    /// <summary>
    /// Tracks the GPU time spent rendering a camera.
    /// For stereo rendering sampling is made from the beginning of the left eye to the end of the right eye.
    /// </summary>
    public class GpuTimingCamera : MonoBehaviour
    {
        [SerializeField]
        private string timingTag = "Frame";

        private Camera timingCamera;

        private void Awake()
        {
            timingCamera = GetComponent<Camera>();
            Debug.Assert(timingCamera, "GpuTimingComponent must be attached to a Camera.");

            if (timingCamera == null)
            {
                enabled = false;
            }
        }

        protected void OnPreRender()
        {
            if (timingCamera.stereoActiveEye == Camera.MonoOrStereoscopicEye.Left || timingCamera.stereoActiveEye == Camera.MonoOrStereoscopicEye.Mono)
            {
                GpuStats.BeginSample(timingTag);
            }
        }

        protected void OnPostRender()
        {
            if (timingCamera.stereoActiveEye == Camera.MonoOrStereoscopicEye.Right || timingCamera.stereoActiveEye == Camera.MonoOrStereoscopicEye.Mono)
            {
                GpuStats.EndSample();
            }
        }
    }
}