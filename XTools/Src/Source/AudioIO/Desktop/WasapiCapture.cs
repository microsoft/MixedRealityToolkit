using System;
using System.Collections.Generic;
using System.Text;
using AudioIO_DLL.Core;
using AudioIO_DLL.Core.Interfaces;
using AudioIO_DLL.IO;
using System.Threading;
using System.Diagnostics;
using System.Runtime.InteropServices;
using AudioIO_DLL.Utils;

namespace AudioIO_DLL.IO
{
    /// <summary>
    /// Audio Capture using Wasapi
    /// See http://msdn.microsoft.com/en-us/library/dd370800%28VS.85%29.aspx
    /// </summary>
    public class WasapiCapture
    {
        private const long REFTIMES_PER_SEC = 10000000;
        private const long REFTIMES_PER_MILLISEC = 10000;
        //private volatile bool stop;
        //private byte[] recordBuffer;
        private readonly string device;
        //private int bytesPerFrame;
        private WaveFormat waveFormat;

        /// <summary>
        /// Indicates recorded data is available 
        /// </summary>
        //public event EventHandler<WaveInEventArgs> DataAvailable;

        /// <summary>
        /// Indicates that all recorded data has now been received.
        /// </summary>
        //public event EventHandler<StoppedEventArgs> RecordingStopped;
        //private int latencyMilliseconds;

        private EventWriterDLL EventWriterDLL = new EventWriterDLL();

        /// <summary>
        /// Initialises a new instance of the WASAPI capture class
        /// </summary>
        public WasapiCapture() :
            this(GetDefaultCaptureDevice())
        {
        }

        /// <summary>
        /// Initialises a new instance of the WASAPI capture class
        /// </summary>
        /// <param name="device">Capture device to use</param>
        public WasapiCapture(string device)
        {
            this.device = device;

            this.EventWriterDLL.Initialize("WasapiCapture on: " + device);
        }

        /// <summary>
        /// Recording wave format
        /// </summary>
        public virtual WaveFormat WaveFormat 
        {
            get { return waveFormat; }
            set { waveFormat = value; }
        }

        /// <summary>
        /// Way of enumerating all the audio capture devices available on the system
        /// </summary>
        /// <returns></returns>
        //public async static Task<IEnumerable<DeviceInformation>> GetCaptureDevices()
        //{
        //    var audioCaptureSelector = MediaDevice.GetAudioCaptureSelector();

        //    // (a PropertyKey)
        //    var supportsEventDrivenMode = "{1da5d803-d492-4edd-8c23-e0c0ffee7f0e} 7";

        //    var captureDevices = await DeviceInformation.FindAllAsync(audioCaptureSelector, new[] { supportsEventDrivenMode } );
        //    foreach (var dev in captureDevices)
        //    {
        //        Debug.WriteLine(dev.ToString());
        //    }
        //    return captureDevices;
        //}

        /// <summary>
        /// Gets the default audio capture device
        /// </summary>
        /// <returns>The default audio capture device</returns>
        public static string GetDefaultCaptureDevice()
        {
            //var defaultCaptureDeviceId = MediaDevice.GetDefaultAudioCaptureId(AudioDeviceRole.Default);
            //return defaultCaptureDeviceId;
            return "";
        }

        /// <summary>
        /// Start Recording
        /// </summary>
        public void StartRecording()
        {
        }

        /// <summary>
        /// Stop Recording
        /// </summary>
        public void StopRecording()
        {
        }

        /// <summary>
        /// Dispose
        /// </summary>
        public void Dispose()
        {
            StopRecording();
        }
    }
}
