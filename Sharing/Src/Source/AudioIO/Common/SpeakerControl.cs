using AudioIO_DLL.IO;
using AudioIO_DLL.Utils;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;


namespace AudioIO_DLL
{
    public class SpeakerControl
    {
        private EventWriterDLL EventWriterDLL = new EventWriterDLL();
        private bool inSession = false;
        private SpeakerInternal speakerInternal = new SpeakerInternal();

        private Stopwatch sw = Stopwatch.StartNew();
        private Stopwatch swBig = Stopwatch.StartNew();
        private uint writesPerSecond = 0;
        private uint dataRecvd = 0;

        /// <summary>
        /// Speaker control constructor
        /// </summary>
        public SpeakerControl()
        {
            this.EventWriterDLL.Initialize("SpeakerControl");
        }

        /// <summary>
        /// Start up the speaker output
        /// </summary>
        public void StartSpeakers()
        {
            if (this.inSession)
            {
                return;
            }
            this.inSession = true;
            this.speakerInternal.Start();
        }

        /// <summary>
        ///  Stop the speaker output
        /// </summary>
        public void StopSpeakers()
        {
            this.speakerInternal.Stop();
            this.inSession = false;
        }

        /// <summary>
        /// Write data into the speaker playing buffer.  The count should be a 
        /// multiple of 4 since these are really floats.
        /// </summary>
        /// <param name="count"></param>
        /// <param name="data"></param>
        public void WriteDataToSpeakers(byte[] data, int offset, int count)
        {
            dataRecvd += (uint)data.Length;
            writesPerSecond++; 
            if (sw.ElapsedMilliseconds > 1000)
            {
                // get a volume to record
                float averageAmplitude = 0.0f;
                for (int sample = 0; sample < count / 4; sample++)
                {
                    float amp = BitConverter.ToSingle(data, sample * 4);
                    averageAmplitude += Math.Abs(amp);
                }
                float Volume = 4 * averageAmplitude / count;

                this.EventWriterDLL.BuildLine("writesPerSecond: " + writesPerSecond + ", dataBytes: " + dataRecvd + ", volume: " + Volume);
                writesPerSecond = 0;
                dataRecvd = 0;
#if WSA81
                sw.Restart();
#endif
            }

            if (swBig.ElapsedMilliseconds > 10000)
            {
                this.EventWriterDLL.FlushBuildString(EventWriterDLL.SeverityTypes.Information, 0x01);
#if WSA81
                swBig.Restart();
#endif
            }

            this.speakerInternal.WriteBytesFromNetwork(data, offset, count);
        }        
        
        /// <summary>
        /// A simple bool representing the running state of the speaker output.
        /// </summary>
        public bool IsRunning
        {
            get { return this.speakerInternal.IsRunning; }
        }
    }
}
