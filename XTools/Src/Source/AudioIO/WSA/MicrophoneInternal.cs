using AudioIO_DLL.Utils;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel.Core;
using Windows.UI.Core;

namespace AudioIO_DLL.IO
{
    /// <summary>
    /// WSA version of the Microphone internal controller!
    /// </summary>
    class MicrophoneInternal : CircularBuffer
    {
        public delegate void DataCallback(byte[] data);
        public DataCallback Callback;

        private WasapiCapture waveIn;
        private byte[] sendBuffer;

        private Object startStopLock;
        private bool startedCapturing = false;

        /// <summary>
        /// The current or last calculated volume
        /// </summary>
        public float Volume
        {
            get;
            private set;
        }

        private EventWriterDLL EventWriterDLL = new EventWriterDLL();

        /// <summary>
        /// MicrophoneInternal constructor, init the circular buffer size. size must be a multiple of 4 
        /// since it really holds floats, store is 1/4 second.
        /// </summary>
        public MicrophoneInternal()
            : base(BufferConstants.QuarterSecond)
        {
            startStopLock = new Object();
            this.EventWriterDLL.Initialize("AudioIO.IO::MicrophoneInternal");
        }

        /// <summary>
        /// Start the Microphone
        /// </summary>
        async public void Start()
        {
            bool shouldStart = false;

            lock (startStopLock)
            {
                this.Reset();

                // don't try to restart over an existing session, call Stop first
                if (!this.startedCapturing)
                {
                    this.startedCapturing = true;
                    shouldStart = true;  // can't do the await inside the lock
                }
            }

            if (shouldStart)
            {
                this.waveIn = new WasapiCapture(); // gets the default capture device
                this.waveIn.DataAvailableEventHandler += CaptureDataAvailable;
                await CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(CoreDispatcherPriority.Normal,
                () =>
                {
                    this.waveIn.StartRecording();
                });

                this.EventWriterDLL.FlushBuildString(EventWriterDLL.SeverityTypes.Information, 0x01);
                await Task.Delay(10);
            }
        }

        /// <summary>
        /// Stop the Microphone.
        /// </summary>
        async public void Stop()
        {
            lock (startStopLock)
            {
                if (this.waveIn != null)
                {
                    this.waveIn.StopRecording();
                    this.waveIn = null;
                }
                this.startedCapturing = false;
            }
            await Task.Delay(10);
            this.Reset();
        }

        // WasapiCapture calls this callback with new audio data
        private void CaptureDataAvailable(object sender, WaveInEventArgs wiea)
        {
            WriteBytes(wiea.Buffer, 0, wiea.BytesRecorded);
        }

        /// <summary>
        /// Checks to see if there is data in the buffer to send.
        /// </summary>
        public void CheckAndSend()
        {
            // if Count is too large, there really isn't any reason to send all of that data, it is too old,
            // so first, read out of the buffer a bit until Count is less than a quarter second
            if (this.sendBuffer == null || (this.ByteCount > BufferConstants.QuarterSecond && this.sendBuffer.Length != BufferConstants.UnknownBufferValue01))
            {
                this.sendBuffer = new byte[BufferConstants.UnknownBufferValue01];
            }

            while (this.ByteCount > BufferConstants.QuarterSecond)
            {
                this.sendBuffer = new byte[BufferConstants.UnknownBufferValue01];
                ReadBytes(this.sendBuffer, 0, BufferConstants.UnknownBufferValue01);
            }

            while (this.ByteCount > 0)
            {
                // then we have data to send
                int bytes = this.ByteCount;
                if (bytes > BufferConstants.UnknownBufferValue01)
                {
                    // this avoids UDP network fragmentation and sends the biggest packet possible without intervention
                    bytes = BufferConstants.UnknownBufferValue01;
                }

                if (this.sendBuffer.Length != bytes)
                {
                    this.sendBuffer = new byte[bytes];
                }

                ReadBytes(this.sendBuffer, 0, bytes);

                if (this.Callback != null)
                {
                    this.Callback(this.sendBuffer);
                }
            }
        }
    }
}
