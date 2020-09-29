// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

namespace Microsoft.MixedReality.Toolkit.Utilities
{
    /// <summary>
    /// Encapsulates access to GPU timing methods.
    /// </summary>
    public static class GpuStats
    {
        [DllImport("GpuStats")]
        private static extern IntPtr GetRenderEventFunc();

        [DllImport("GpuStats")]
        private static extern double GetGpuDuration(int eventId);

        [DllImport("GpuStats")]
        private static extern ulong GetVramUse();

        private const int BaseBeginEventId = 1000;
        private const int BaseEndEventId = 2000;

        private static readonly Stack<int> CurrentEventId = new Stack<int>();
        private static readonly Dictionary<string, int> EventIds = new Dictionary<string, int>();

        private static int nextAvailableEventId = 0;

        /// <summary>
        /// Gets the latest available sample duration for the given event.
        /// </summary>
        /// <param name="eventId">Name of the event.</param>
        /// <param name="duration">The sample duration in seconds.</param>
        /// <returns>Whether the query result is valid, the query was disjoint, or the event ID was not found.</returns>
        public static GpuDurationResult GetSampleDuration(string eventId, out double duration)
        {
            if (EventIds.TryGetValue(eventId, out int eventValue))
            {
                var result = GetGpuDuration(eventValue);
                if (result < -1.0)
                {
                    duration = double.NaN;
                    return GpuDurationResult.NotFound;
                }

                if (result < 0.0)
                {
                    duration = double.NaN;
                    return GpuDurationResult.Disjoint;
                }

                duration = result;
                return GpuDurationResult.Valid;
            }

            duration = double.NaN;
            return GpuDurationResult.NotFound;
        }

        /// <summary>
        /// Gets the latest queried VRAM usage.
        /// </summary>
        /// <remarks>Uses DXGI_QUERY_VIDEO_MEMORY_INFO for this data.</remarks>
        /// <returns>The VRAM usage in bytes.</returns>
        public static ulong GetVideoMemoryUsage() => GetVramUse();

        /// <summary>
        /// Begins sampling GPU time.
        /// </summary>
        /// <param name="eventId">Name of the event.</param>
        /// <returns>Whether a <see cref="BeginSample"/> with the same event name was added.</returns>
        public static bool BeginSample(string eventId)
        {
            if (!EventIds.TryGetValue(eventId, out int eventValue))
            {
                if (nextAvailableEventId == BaseEndEventId)
                {
                    return false;
                }

                eventValue = nextAvailableEventId;
                EventIds.Add(eventId, nextAvailableEventId++);
            }

            if (CurrentEventId.Contains(eventValue))
            {
                Debug.LogWarning("BeginSample() is being called again without a corresponding EndSample() call.");
                return false;
            }

            CurrentEventId.Push(eventValue);

            // Begin measuring GPU time
            int eventFunctionId = eventValue + BaseBeginEventId;
            GL.IssuePluginEvent(GetRenderEventFunc(), eventFunctionId);
            return true;
        }

        /// <summary>
        /// Ends the GPU sample currently in flight.
        /// </summary>
        public static void EndSample()
        {
            if (CurrentEventId.Count > 0)
            {
                // End measuring GPU frame time
                int eventId = CurrentEventId.Pop() + BaseEndEventId;
                GL.IssuePluginEvent(GetRenderEventFunc(), eventId);
            }
        }
    }
}
