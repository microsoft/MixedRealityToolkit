using AudioIO_DLL.Utils;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AudioIO_DLL.IO
{
    /// <summary>
    /// Desktop version of the Microphone internal controller!
    /// </summary>
    class MicrophoneInternal : CircularBuffer
    {
        public delegate void DataCallback(byte[] data);
        public DataCallback Callback;

        private WasapiCapture waveIn;
        private byte[] sendBuffer;

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
            this.EventWriterDLL.Initialize("AudioIO.IO::MicrophoneInternal");

            //var t = Task.Run(() => startMicrophoneTaskAndRun());
        }

        /// <summary>
        /// Start the Microphone
        /// </summary>
        public void Start()
        {
            this.waveIn = new WasapiCapture(); // gets the default capture device
            //this.waveIn.DataAvailable += CaptureDataAvailable; 
            //await CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(CoreDispatcherPriority.Normal,
            //() =>
            //{
            //    this.waveIn.StartRecording();
            //});

            //this.startedCapturing = true;

            //await Task.Delay(100);
        }

        /// <summary>
        /// Stop the Microphone.
        /// </summary>
        public void Stop()
        {
           // this.startedCapturing = false;
            this.waveIn.StopRecording();
        }
        
        // WasapiCapture calls this callback with new audio data
        private void CaptureDataAvailable(object sender, WaveInEventArgs wiea)
        {
            WriteBytes(wiea.Buffer, 0, wiea.BytesRecorded);

            // calculate the volume over the whole buffer (which is 1/10th of a second)...

        }

        //private async Task<bool> startMicrophoneTaskAndRun()
        //{
        //    try
        //    {
        //        while (!this.finishLooping)
        //        {
        //            if (this.startedCapturing)
        //            {
        //                CheckAndSend();
        //            }
        //            await Task.Yield();
        //            await Task.Delay(1);
        //        }
        //    }
        //    catch(Exception ex)
        //    {
        //        this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "Exception in AudioSource: " + ex.ToString());
        //    }
        //    return true;
        //}

        /// <summary>
        /// Checks to see if there is data in the buffer to send.
        /// </summary>
        public void CheckAndSend()
        {
            while (this.ByteCount > 0)
            {
                // then we have data to send
                int bytes = this.ByteCount;
                if (bytes > BufferConstants.UnknownBufferValue01)
                {
                    // this avoids network fragmentation and sends the biggest packet possible without intervention
                    bytes = BufferConstants.UnknownBufferValue01;
                }

                // TODO: make sure this is the preferred way to make a buffer to send in RakNet...
                this.sendBuffer = new byte[bytes];
                ReadBytes(this.sendBuffer, 0, bytes);

///////////////////////////////////////////////                    SendOutgoingData(this.sendBuffer);
            }
        }
    }
}
