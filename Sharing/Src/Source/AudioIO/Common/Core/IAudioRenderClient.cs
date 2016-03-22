using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace AudioIO_DLL.Core.Interfaces
{
    [ComImport, Guid("F294ACFC-3146-4483-A7BF-ADDCA7C260E2"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    interface IAudioRenderClient
    {
        int GetBuffer(int numFramesRequested, out IntPtr dataBufferPointer);
        int ReleaseBuffer(int numFramesWritten, AudioClientBufferFlags bufferFlags);
    }

    // This class is here to ensure .Net native includes the IAudioRenderClient interface
    // It does not need to be used anywhere; just declared.
    class AudioRenderClientImpl : IAudioRenderClient
    {
        public int GetBuffer(int numFramesRequested, out IntPtr dataBufferPointer)
        {
            throw new NotImplementedException();
        }

        public int ReleaseBuffer(int numFramesWritten, AudioClientBufferFlags bufferFlags)
        {
            throw new NotImplementedException();
        }
    }
}
