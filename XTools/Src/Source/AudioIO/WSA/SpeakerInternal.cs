using AudioIO_DLL.Core;
using AudioIO_DLL.Utils;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel.Core;
using Windows.UI.Core;

namespace AudioIO_DLL.IO
{
    public class SpeakerInternal : HRTFBufferDLL
    {
        private WasapiOut waveOut;
        // the accumulatorBuffer accumulates incoming audio bytes, after enough
        // are accumulated, we do the HRTF processing and the output is a stereo
        // stream.
        private CircularBuffer accumulatorBuffer = new CircularBuffer(BufferConstants.QuarterSecond);
        private readonly object lockObjectCircularBuffer;
        private bool stopRequestPending = false;
        private bool started = false;
        private bool startRequestPending = false;
        private Object startStopLock;

        private EventWriterDLL EventWriterDLL = new EventWriterDLL();

        /// <summary>
        /// SpeakerInternal constructor, init the circular buffer (must be a multiple of 4 since the 
        /// bytes are really storing floats, store 1/4 second.
        /// </summary>
        public SpeakerInternal()
        {
            this.EventWriterDLL.Initialize("Audio.IO.SpeakerInternal");
            this.lockObjectCircularBuffer = new object();
            this.startStopLock = new Object();
        }

        /// <summary>
        /// Start the speaker processing Task
        /// </summary>
        public void Start()
        {
            lock(this.startStopLock)
            {
                if (this.stopRequestPending)
                {
                    this.startRequestPending = true;
                }
                else if (!this.started)
                {
                    this.started = true;
                    var t = Task.Run(() => StartPlayingToSpeaker());
                }
            }
        }

        /// <summary>
        /// Start the Microphone
        /// </summary>
        async private void StartPlayingToSpeaker()
        {
            try
            {
                this.startRequestPending = false;

                SetWaveFormat(BufferConstants.BaseFrequency, BufferConstants.AudioChannelsAfterHRTF);
                this.waveOut = new WasapiOut(AudioClientShareMode.Shared, BufferConstants.AudioBufferLatency);
                await this.waveOut.Init(this);
                this.waveOut.Play();

                this.EventWriterDLL.BuildLine("+15 start => waveOut created and we are spinning now!!!");
                this.EventWriterDLL.FlushBuildString(EventWriterDLL.SeverityTypes.Information, 0x01);

                while (!this.stopRequestPending)
                {
                    // audio plays on callbacks, but HRTF processing needsd to happen here
                    while (this.accumulatorBuffer.FloatCount > BufferConstants.HRTFBufferDLLSize)
                    {
                        lock (lockObjectCircularBuffer)
                        {
                            // grab HRTFBufferDLL.HRTFBufferDLLSize mono floats and send to hrtf for processing
                            ReadFromAccumulator(this.accumulatorBuffer);
                        }

                        // do the HRTF processing
                        ProcessHRTF();
                    }

                    // be polite to other threads and don't use 100% of the available CPU time...
                    await Task.Delay(3);

                    lock (this.startStopLock)
                    {
                        // if a new stop and new start request are both pending, then cancel them out and keep going.
                        if (this.stopRequestPending && this.startRequestPending)
                        {
                            this.stopRequestPending = false;
                            this.startRequestPending = false;
                        }
                    }
                }

                // Now process stop request

                this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Warning, 0x01, "-1 Stop => waveOut stopping!!!");
                if (this.waveOut != null)
                {
                    this.waveOut.Stop();
                    this.Reset();
                }
                this.stopRequestPending = false;
                this.started = false;
                if (this.startRequestPending)
                {
                    Start();
                }
            }
            catch (Exception ex)
            {
                this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "Exception in SpeakerInternal: " + ex.ToString());
            }
        }

        /// <summary>
        /// Stop the Microphone.
        /// </summary>
        public void Stop()
        {
            lock(this.startStopLock)
            {
                this.startRequestPending = false;
                if (this.started)
                {
                    this.stopRequestPending = true;
                }
            }
        }


        /// <summary>
        /// Write bytes received from the network to the accumulator buffer.  
        /// </summary>
        /// <param name="data">Data to write</param>
        /// <param name="offset">Offset into data</param>
        /// <param name="count">Number of bytes to write</param>
        /// <returns>number of bytes written</returns>
        public int WriteBytesFromNetwork(byte[] data, int offset, int count)
        {
            lock (lockObjectCircularBuffer)
            {
                return this.accumulatorBuffer.WriteBytes(data, offset, count);
            }
        }        
        
        /// <summary>
        /// A simple bool representing the running state of the speaker output.
        /// </summary>
        public bool IsRunning
        {
            get { return this.started; }
        }
    }
}
