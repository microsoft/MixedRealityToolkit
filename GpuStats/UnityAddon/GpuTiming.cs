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
        private static extern double GetLastFrameGPUTime(int eventId);

        private const int BaseBeginEventId = 1000;
        private const int BaseEndEventId = 2000;

        private static readonly Stack<int> CurrentEventId = new Stack<int>();
        private static readonly Dictionary<string, int> EventIds = new Dictionary<string, int>();

        private static int nextAvailableEventId = 0;

        /// <summary>
        /// Gets the latest available sample time for the given event.
        /// </summary>
        /// <param name="eventId">Name of the event.</param>
        /// <returns>Time in milliseconds.</returns>
        public static double GetTime(string eventId)
        {
            int eventValue;
            if (EventIds.TryGetValue(eventId, out eventValue))
            {
                return GetLastFrameGPUTime(eventValue);
            }

            return -1;
        }

        /// <summary>
        /// Begins sampling GPU time.
        /// </summary>
        /// <param name="eventId">Name of the event.</param>
        /// <returns>Returns true if a BeginSample with the same event name was last added.</returns>
        public static bool BeginSample(string eventId)
        {
            int eventValue;
            if (!EventIds.TryGetValue(eventId, out eventValue))
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
                Debug.LogWarning("BeginSample() is being called without a corresponding EndSample() call.");
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
