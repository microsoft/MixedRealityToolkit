using System;
using AudioIO_DLL.Core.Interfaces;
using System.Runtime.InteropServices;
using AudioIO_DLL.Utils;

namespace AudioIO_DLL.Core
{
    /// <summary>
    /// Audio Render Client
    /// </summary>
    public class AudioRenderClient : IDisposable
    {
        private EventWriterDLL EventWriterDLL = new EventWriterDLL();

        IAudioRenderClient audioRenderClientInterface;

        internal AudioRenderClient(IAudioRenderClient audioRenderClientInterface)
        {
            this.EventWriterDLL.Initialize("AudioRenderClient");

            this.audioRenderClientInterface = audioRenderClientInterface;
        }

        /// <summary>
        /// Gets a pointer to the buffer
        /// </summary>
        /// <param name="numFramesRequested">Number of frames requested</param>
        /// <returns>Pointer to the buffer</returns>
        public IntPtr GetBuffer(int numFramesRequested)
        {
            IntPtr bufferPointer;
            int hresult = audioRenderClientInterface.GetBuffer(numFramesRequested, out bufferPointer);
            if (hresult != 0)
            {
                this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "Error Code in AudioRenderClient::GetBuffer: " + hresult);
            }
            return bufferPointer;
        }

        /// <summary>
        /// Release buffer
        /// </summary>
        /// <param name="numFramesWritten">Number of frames written</param>
        /// <param name="bufferFlags">Buffer flags</param>
        public void ReleaseBuffer(int numFramesWritten,AudioClientBufferFlags bufferFlags)
        {
            int hresult = audioRenderClientInterface.ReleaseBuffer(numFramesWritten, bufferFlags);
            if (hresult != 0)
            {
                this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "Error Code in AudioRenderClient::ReleaseBuffer: " + hresult);
            }
        }

        #region IDisposable Members

        /// <summary>
        /// Release the COM object
        /// </summary>
        public void Dispose()
        {
            if (audioRenderClientInterface != null)
            {
                // althugh GC would do this for us, we want it done now
                // to let us reopen WASAPI
                Marshal.ReleaseComObject(audioRenderClientInterface);
                audioRenderClientInterface = null;
                GC.SuppressFinalize(this);
            }
        }

        #endregion
    }
}
