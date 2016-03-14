using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using AudioIO_DLL.Core;
using AudioIO_DLL.Core.Interfaces;
using AudioIO_DLL.IO;
using AudioIO_DLL.Utils;

namespace AudioIO_DLL.IO
{
    /// <summary>
    /// WASAPI Out for Windows RT
    /// </summary>
    public class WasapiOut 
    {
        //private AudioClient audioClient;
        private readonly string device;
        private readonly AudioClientShareMode shareMode;
        //private AudioRenderClient renderClient;
        //private IWaveProvider sourceProvider;
        private int latencyMilliseconds;
        //private int bufferFrameCount;
        //private int bytesPerFrame;
        //private byte[] readBuffer;
        private volatile PlaybackState playbackState;
        //private WaveFormat outputFormat;
        //private bool resamplerNeeded;
        //private IntPtr frameEventWaitHandle;
        private readonly SynchronizationContext syncContext;

        private EventWriterDLL EventWriterDLL = new EventWriterDLL();
        //private bool waitingForInitToFinish = true;
        //private uint maxNumberOfFailedWaits = 10;

        /// <summary>
        /// Playback Stopped
        /// </summary>
        //public event EventHandler<StoppedEventArgs> PlaybackStopped;

        /// <summary>
        /// WASAPI Out using default audio endpoint
        /// </summary>
        /// <param name="shareMode">ShareMode - shared or exclusive</param>
        /// <param name="latency">Desired latency in milliseconds</param>
        public WasapiOut(AudioClientShareMode shareMode, int latency)
        {
            this.EventWriterDLL.Initialize("AudioIO.IO::WasapiOut");
        }

        /// <summary>
        /// Creates a new WASAPI Output
        /// </summary>
        /// <param name="device">Device to use</param>
        /// <param name="shareMode"></param>
        /// <param name="latency"></param>
        public WasapiOut(string device, AudioClientShareMode shareMode, int latency)
        {
            this.device = device;
            this.shareMode = shareMode;
            this.latencyMilliseconds = latency;
            this.syncContext = SynchronizationContext.Current;
        }


        #region IWavePlayer Members

        /// <summary>
        /// Begin Playback
        /// </summary>
        public void Play()
        {
        }

        /// <summary>
        /// Stop playback and flush buffers
        /// </summary>
        public void Stop()
        {
        }

        /// <summary>
        /// Stop playback without flushing buffers
        /// </summary>
        public void Pause()
        {
            if (playbackState == PlaybackState.Playing)
            {
                playbackState = PlaybackState.Paused;
            }

        }

        /// <summary>
        /// Init the output audio device
        /// </summary>
        /// <param name="waveProvider">waveProvider interface</param>
        public void Init(IWaveProvider waveProvider)
        {
        }

        //IWaveProvider waveProviderTemp;

        /// <summary>
        /// Playback State
        /// </summary>
        public PlaybackState PlaybackState
        {
            get { return playbackState; }
        }

        #endregion

        #region IDisposable Members

        /// <summary>
        /// Dispose
        /// </summary>
        public void Dispose()
        {
        }

        #endregion
    }
}
