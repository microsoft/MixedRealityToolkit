// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.MixedReality.Toolkit.Utilities
{
    public enum GpuDurationResult
    {
        /// <summary>
        /// The duration data is valid and usable.
        /// </summary>
        Valid,
        /// <summary>
        /// Something occurred in between BeginSample and EndSample that caused the timestamp counter to become discontinuous or disjoint.
        /// Examples include unplugging the AC cord on a laptop, overheating, or throttling up/down due to laptop savings events.
        /// </summary>
        Disjoint,
        /// <summary>
        /// The frame event wasn't found.
        /// </summary>
        NotFound,
    }
}
