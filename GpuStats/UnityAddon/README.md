# GpuStats

Use the GpuStats class to measure the time of your GPU calls.

Add the GpuTimingCamera component to the camera you want to track the total GPU cost of. Use `GpuStats.GetSampleDuration("Frame", out double duration)` to get the time (where "Frame" is the default tag used by GpuTimingCamera), or register for the GpuTimingCamera.NewGpuFrameDuration event in the inspector or in code.
