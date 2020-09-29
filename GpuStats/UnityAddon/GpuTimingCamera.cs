// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

using UnityEngine;
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
        [SerializeField]
        private string timingTag = "Frame";

        private Camera timingCamera;

        protected void Start()
        {
            timingCamera = GetComponent<Camera>();
            Debug.Assert(timingCamera, "GpuTimingCamera component must be attached to a Camera");
        }

        protected void OnPreRender()
        {
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
}
