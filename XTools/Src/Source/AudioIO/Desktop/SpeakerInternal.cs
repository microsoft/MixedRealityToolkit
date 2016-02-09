using AudioIO_DLL.Core;
using AudioIO_DLL.Utils;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;


namespace AudioIO_DLL.IO
{
    /// <summary>
    /// Desktop version of the Speaker internal interfaces
    /// </summary>
    public class SpeakerInternal : HRTFBufferDLL
    {
        // the accumulatorBuffer accumulates incoming audio bytes, after enough
        // are accumulated, we do the HRTF processing and the output is a stereo
        // stream.
        private CircularBuffer accumulatorBuffer = new CircularBuffer(BufferConstants.QuarterSecond);

        private EventWriterDLL EventWriterDLL = new EventWriterDLL();

        /// <summary>
        /// SpeakerInternal constructor, init the circular buffer (must be a multiple of 4 since the 
        /// bytes are really storing floats, store 1/4 second.
        /// </summary>
        public SpeakerInternal()
        {
            this.EventWriterDLL.Initialize("Audio.IO.SpeakerInternal");
        }

        public void Start()
        {
        }

        /// <summary>
        /// Stop the Microphone.
        /// </summary>
        public void Stop()
        {
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
            return this.accumulatorBuffer.WriteBytes(data, offset, count);
        }

        public bool IsRunning
        {
            get { return false; }
        }
    }
}
