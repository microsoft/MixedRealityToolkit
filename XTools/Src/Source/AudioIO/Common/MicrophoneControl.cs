using AudioIO_DLL.IO;
using AudioIO_DLL.Utils;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;

namespace AudioIO_DLL
{
    public class MicrophoneControl
    {
        /// <summary>
        /// Delegate for microphone data callbacks
        /// </summary>
        /// <param name="count"></param>
        /// <param name="data"></param>
        public delegate void MicrophoneDataCallback(int count, byte[] data);
        public MicrophoneDataCallback Callback;

        private EventWriterDLL EventWriterDLL = new EventWriterDLL();
        private bool inSession = false;
        private MicrophoneInternal microphoneInternal = new MicrophoneInternal();

        private Stopwatch sw = Stopwatch.StartNew();
        private Stopwatch swBig = Stopwatch.StartNew();
        private uint readsPerSecond = 0;
        private uint dataSent = 0;
        
        /// <summary>
        /// Microphone control constructor
        /// </summary>
        public MicrophoneControl()
        {
            this.EventWriterDLL.Initialize("MicrophoneControl");
        }

        /// <summary>
        /// Connect the audio to the server session identified by the ID.
        /// </summary>
        /// <param name="SessionID">The session identifier used to establish communications with the server</param>
        /// <returns>return true if succeeded, false if not</returns>
        public bool ConnectToSession(string SessionID)
        {
            // no double joins...
            if (this.inSession)
            {
                return false;
            }

            this.inSession = true;

            // other code has already started the session and set up communications so all we need to 
            // do here is turn on the Mic and set the correct information for the network

            StartMicrophone();

            return false;
        }

        /// <summary>
        /// Disconnect from the existing session.  We already have the session ID in storage. 
        /// </summary>
        /// <returns>return true if succeeded, false if not</returns>
        public bool DisconnectFromSession()
        {
            StopMicrophone();

            this.inSession = false;

            return false;
        }

        public void CheckAndSend()
        {
            this.microphoneInternal.CheckAndSend();
        }

        private void StartMicrophone()
        {
            this.microphoneInternal.Callback += this.SendMicrophoneData;
            this.microphoneInternal.Start();
        }

        private void StopMicrophone()
        {
            this.microphoneInternal.Stop();
            this.microphoneInternal.Callback -= this.SendMicrophoneData;
        }

        private void SendMicrophoneData(byte[] data)
        {
            if (this.Callback != null)
            {
                dataSent += (uint)data.Length;
                readsPerSecond++; 
                if (sw.ElapsedMilliseconds > 1000)
                {
                    // get a volume to record
                    float averageAmplitude = 0.0f;
                    for (int sample = 0; sample < data.Length / 4; sample++)
                    {
                        float amp = BitConverter.ToSingle(data, sample * 4);
                        averageAmplitude += Math.Abs(amp);
                    }
                    float Volume = 4 * averageAmplitude / data.Length;

                    this.EventWriterDLL.BuildLine("readsPerSecond: " + readsPerSecond + ", dataBytes: " + dataSent + ", volume: " + Volume);
                    readsPerSecond = 0;
                    dataSent = 0;
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

                this.Callback(data.Length, data);
            }
        }
    }
}
