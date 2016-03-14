using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace AudioIO_DLL.Core.Interfaces
{
    [ComImport, Guid("C8ADBD64-E71E-48a0-A4DE-185C395CD317"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    interface IAudioCaptureClient
    {
        int GetBuffer(
            out IntPtr dataBuffer, 
            out int numFramesToRead, 
            out AudioClientBufferFlags bufferFlags,
            out long devicePosition,
            out long qpcPosition);

        int ReleaseBuffer(int numFramesRead);

        int GetNextPacketSize(out int numFramesInNextPacket);
    }

    // This class is here to ensure .Net native includes the IAudioCaptureClient interface
    // It does not need to be used anywhere; just declared.
    class AudioCaptureClientImpl : IAudioCaptureClient
    {
        public int GetBuffer(out IntPtr dataBuffer, out int numFramesToRead, out AudioClientBufferFlags bufferFlags, out long devicePosition, out long qpcPosition)
        {
            throw new NotImplementedException();
        }

        public int ReleaseBuffer(int numFramesRead)
        {
            throw new NotImplementedException();
        }

        public int GetNextPacketSize(out int numFramesInNextPacket)
        {
            throw new NotImplementedException();
        }
    }
}
