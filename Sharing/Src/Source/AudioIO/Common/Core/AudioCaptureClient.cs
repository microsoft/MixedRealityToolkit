using System;
using System.Runtime.InteropServices;
using AudioIO_DLL.Core.Interfaces;
using AudioIO_DLL.Utils;

namespace AudioIO_DLL.Core
{
    /// <summary>
    /// Audio Capture Client
    /// </summary>
    public class AudioCaptureClient : IDisposable
    {
        private EventWriterDLL EventWriterDLL = new EventWriterDLL();

        private IAudioCaptureClient audioCaptureClientInterface;

        internal AudioCaptureClient(IAudioCaptureClient audioCaptureClientInterface)
        {
            this.EventWriterDLL.Initialize("AudioCaptureClient");

            this.audioCaptureClientInterface = audioCaptureClientInterface;
        }

        /// <summary>
        /// Gets a pointer to the buffer
        /// </summary>
        /// <param name="numFramesToRead">Number of frames to read</param>
        /// <param name="bufferFlags">Buffer flags</param>
        /// <returns>Pointer to the buffer</returns>
        public IntPtr GetBuffer(
            out int numFramesToRead,
            out AudioClientBufferFlags bufferFlags)
        {
            IntPtr bufferPointer;
            long devicePosition;
            long qpcPosition;
            int hresult = audioCaptureClientInterface.GetBuffer(out bufferPointer, out numFramesToRead, out bufferFlags, out devicePosition, out qpcPosition);
            if (hresult != 0)
            {
                this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "Error Code in AudioCaptureClient::GetBuffer: " + hresult);
            }
            return bufferPointer;
        }

        /// <summary>
        /// Gets the size of the next packet
        /// </summary>
        public int GetNextPacketSize()
        {
            int numFramesInNextPacket;
            int hresult = audioCaptureClientInterface.GetNextPacketSize(out numFramesInNextPacket);
            if (hresult != 0)
            {
                this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "Error Code in AudioCaptureClient::GetNextPacketSize: " + hresult);
            }
            return numFramesInNextPacket;
        }

        /// <summary>
        /// Release buffer
        /// </summary>
        /// <param name="numFramesWritten">Number of frames written</param>
        public void ReleaseBuffer(int numFramesWritten)
        {
            int hresult = audioCaptureClientInterface.ReleaseBuffer(numFramesWritten);
            if (hresult != 0)
            {
                this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "Error Code in AudioCaptureClient::ReleaseBuffer: " + hresult);
            }
        }

        #region IDisposable Members

        /// <summary>
        /// Release the COM object
        /// </summary>
        public void Dispose()
        {
            if (audioCaptureClientInterface != null)
            {
                // althugh GC would do this for us, we want it done now
                // to let us reopen WASAPI
                Marshal.ReleaseComObject(audioCaptureClientInterface);
                audioCaptureClientInterface = null;
                GC.SuppressFinalize(this);
            }
        }

        #endregion
    }
}