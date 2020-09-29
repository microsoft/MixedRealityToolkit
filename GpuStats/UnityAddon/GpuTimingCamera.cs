// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

using System;
using UnityEngine;
using UnityEngine.Events;
using UnityEngine.Rendering;
using UnityEngine.XR;

namespace Microsoft.MixedReality.Toolkit.Utilities
{
    /// <summary>
    /// Tracks the GPU time spent rendering a camera.
    /// For multi-pass stereo rendering, sampling is made from the beginning of the left eye to the end of the right eye.
    /// </summary>
    public class GpuTimingCamera : MonoBehaviour
    {
        [SerializeField, Tooltip("The tag to use with GpuStats.BeginSample.")]
        private string timingTag = "Frame";

        [SerializeField, Tooltip("Fires in OnPreRender with the data from the previous frame.")]
        private UnityGpuFrameDurationEvent newGpuFrameDuration = null;

        /// <summary>
        /// Fires in OnPreRender with the data from the previous frame.
        /// </summary>
        public UnityGpuFrameDurationEvent NewGpuFrameDuration => newGpuFrameDuration;

        private Camera timingCamera;

        protected void Start()
        {
            timingCamera = GetComponent<Camera>();
            Debug.Assert(timingCamera, "GpuTimingCamera component must be attached to a Camera");
        }

        protected void OnPreRender()
        {
            if (newGpuFrameDuration?.GetPersistentEventCount() > 0)
            {
                newGpuFrameDuration.Invoke(GpuStats.GetSampleDuration(timingTag, out double duration), (float)duration);
            }

            if (timingCamera.stereoActiveEye != Camera.MonoOrStereoscopicEye.Right)
            {
                GpuStats.BeginSample(timingTag);
            }
        }

        protected void OnPostRender()
        {
            if (timingCamera.stereoActiveEye != Camera.MonoOrStereoscopicEye.Left
                || (XRSettings.isDeviceActive
                    && (XRSettings.eyeTextureDesc.vrUsage == VRTextureUsage.TwoEyes
                        || XRSettings.eyeTextureDesc.dimension == TextureDimension.Tex2DArray)))
            {
                GpuStats.EndSample();
            }
        }
    }

    /// <summary>
    /// A custom UnityEvent providing a GpuDurationResult and a float duration.
    /// </summary>
    [Serializable]
    public class UnityGpuFrameDurationEvent : UnityEvent<GpuDurationResult, float> { }
}
