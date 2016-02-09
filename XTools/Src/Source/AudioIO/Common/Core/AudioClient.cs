using System;
using AudioIO_DLL.Core.Interfaces;
using System.Runtime.InteropServices;
using AudioIO_DLL.IO;
using AudioIO_DLL.Utils;
#if WSA81
using Windows.ApplicationModel.Core;
using Windows.UI.Core;
#endif

namespace AudioIO_DLL.Core
{
    /// <summary>
    /// Windows Vista CoreAudio AudioClient2
    /// </summary>
    public class AudioClient2 : IDisposable
    {
        private EventWriterDLL EventWriterDLL = new EventWriterDLL();

        IAudioClient2 audioClient2Interface;
        IAudioClient audioClientInterface;
        WaveFormatExtensible mixFormat;
        AudioRenderClient audioRenderClient;
        AudioCaptureClient audioCaptureClient;

        internal AudioClient2(IAudioClient2 audioClient2InterfaceIn)
        {
            this.EventWriterDLL.Initialize("AudioClient2");

            this.audioClient2Interface = audioClient2InterfaceIn;
            this.audioClientInterface = (IAudioClient)audioClient2InterfaceIn;
        }

        /// <summary>
        /// Mix Format,
        /// Can be called before initialize
        /// </summary>
        public WaveFormatExtensible MixFormat
        {
            get
            {
                if (mixFormat.WaveFormatTag == WaveFormatEncoding.Unknown)
                {
                    IntPtr waveFormatPointer;
                    int hresult = audioClientInterface.GetMixFormat(out waveFormatPointer);
                    if (hresult != 0)
                    {
                        this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "Error Code in AudioClient::WaveFormat get: " + hresult);
                    }
                    WaveFormatExtensible waveFormat = WaveFormatExtensible.MarshalFromPtr(waveFormatPointer);
                    Marshal.FreeCoTaskMem(waveFormatPointer);
                    mixFormat = waveFormat;
                    return waveFormat;
                }
                else
                {
                    return mixFormat;
                }
            }
        }

        /// <summary>
        /// Initialize the Audio Client
        /// </summary>
        /// <param name="shareMode">Share Mode</param>
        /// <param name="streamFlags">Stream Flags</param>
        /// <param name="bufferDuration">Buffer Duration</param>
        /// <param name="periodicity">Periodicity</param>
        /// <param name="waveFormat">Wave Format</param>
        /// <param name="audioSessionGuid">Audio Session GUID (can be null)</param>
        public int Initialize(AudioClientShareMode shareMode,
            AudioClientStreamFlags streamFlags,
            long bufferDuration,
            long periodicity,
            ref WaveFormatExtensible waveFormat,
            Guid audioSessionGuid)
        {
            int hresult = 0;
            hresult = audioClientInterface.Initialize(shareMode, streamFlags, bufferDuration, periodicity, ref waveFormat, ref audioSessionGuid);

            if (hresult != 0)
            {
                this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "Error Code in AudioClient::Initialize: " + hresult);
            }
            // may have changed the mix format so reset it
            mixFormat = new WaveFormatExtensible();
            return hresult;
        }

        /// <summary>
        /// Gets the buffer size (must initialize first)
        /// </summary>
        public int BufferSize
        {
            get
            {
                uint bufferSize;
                int hresult = audioClientInterface.GetBufferSize(out bufferSize);
                if (hresult != 0)
                {
                    this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "Error Code in AudioClient::BufferSize get: " + hresult);
                }
                return (int)bufferSize;
            }
        }

        /// <summary>
        /// Gets the stream latency (must initialize first)
        /// </summary>
        public long StreamLatency
        {
            get
            {
                return audioClientInterface.GetStreamLatency();
            }
        }

        /// <summary>
        /// Gets the current padding (must initialize first)
        /// </summary>
        public int CurrentPadding
        {
            get
            {
                int currentPadding;
                int hresult = audioClientInterface.GetCurrentPadding(out currentPadding);
                if (hresult != 0)
                {
                    this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "Error Code in AudioClient::CurrentPadding get: " + hresult);
                }
                return currentPadding;
            }
        }

        // TODO: GetService:
        // IID_IAudioCaptureClient
        // IID_IAudioSessionControl
        // IID_IAudioStreamVolume
        // IID_IChannelAudioVolume
        // IID_ISimpleAudioVolume

        /// <summary>
        /// Gets the AudioRenderClient service
        /// </summary>
        public AudioRenderClient AudioRenderClient
        {
            get
            {
                if (audioRenderClient == null)
                {
                    object audioRenderClientInterface;
                    Guid audioRenderClientGuid = new Guid("F294ACFC-3146-4483-A7BF-ADDCA7C260E2");
                    int hresult = audioClientInterface.GetService(audioRenderClientGuid, out audioRenderClientInterface);
                    if (hresult != 0)
                    {
                        this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "Error Code in AudioClient::AudioRenderClient get: " + hresult);
                    }
                    audioRenderClient = new AudioRenderClient((IAudioRenderClient)audioRenderClientInterface);
                }
                return audioRenderClient;
            }
        }

        /// <summary>
        /// Gets the AudioCaptureClient service
        /// </summary>
        public AudioCaptureClient AudioCaptureClient
        {
            get
            {
                if (audioCaptureClient == null)
                {
                    object audioCaptureClientInterface;
                    Guid audioCaptureClientGuid = new Guid("c8adbd64-e71e-48a0-a4de-185c395cd317");
                    int hresult = audioClientInterface.GetService(audioCaptureClientGuid, out audioCaptureClientInterface);
                    if (hresult != 0)
                    {
                        this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "Error Code in AudioClient::AudioCaptureClient get: " + hresult);
                    }
                    audioCaptureClient = new AudioCaptureClient((IAudioCaptureClient)audioCaptureClientInterface);
                }
                return audioCaptureClient;
            }
        }

        /// <summary>
        /// Determines whether if the specified output format is supported
        /// </summary>
        /// <param name="shareMode">The share mode.</param>
        /// <param name="desiredFormat">The desired format.</param>
        /// <returns>
        /// 	<c>true</c> if [is format supported] [the specified share mode]; otherwise, <c>false</c>.
        /// </returns>
        public bool IsFormatSupported(AudioClientShareMode shareMode,
            WaveFormatExtensible desiredFormat)
        {
            WaveFormatExtensible closestMatchFormat;
            return IsFormatSupported(shareMode, desiredFormat, out closestMatchFormat);
        }



        /// <summary>
        /// Determines if the specified output format is supported in shared mode
        /// </summary>
        /// <param name="shareMode">Share Mode</param>
        /// <param name="desiredFormat">Desired Format</param>
        /// <param name="closestMatchFormat">Output The closest match format.</param>
        /// <returns>
        /// 	<c>true</c> if [is format supported] [the specified share mode]; otherwise, <c>false</c>.
        /// </returns>
        public bool IsFormatSupported(AudioClientShareMode shareMode, WaveFormatExtensible desiredFormat, out WaveFormatExtensible closestMatchFormat)
        {
            int hresult = audioClientInterface.IsFormatSupported(shareMode, ref desiredFormat, out closestMatchFormat);
            // S_OK is 0, S_FALSE = 1
            if (hresult == 0)
            {
                // directly supported
                return true;
            }
            if (hresult == 1)
            {
                return false;
            }
            else if (hresult == unchecked((int)0x88890008)) // UnsupportedFormat
            {
                return false;
            }
            else
            {
                this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "Error Code in AudioClient::IsFormatSupported: " + hresult);
                return false;
            }
        }


        /// <summary>
        /// Starts the audio stream
        /// </summary>
        public void Start()
        {
            audioClientInterface.Start();
        }

        /// <summary>
        /// Stops the audio stream.
        /// </summary>
        public void Stop()
        {
            audioClientInterface.Stop();
        }

        /// <summary>
        /// Set the Event Handle for buffer synchro.
        /// </summary>
        /// <param name="eventWaitHandle">The Wait Handle to setup</param>
        public void SetEventHandle(IntPtr eventWaitHandle)
        {
            audioClientInterface.SetEventHandle(eventWaitHandle);
        }

        /// <summary>
        /// Resets the audio stream
        /// Reset is a control method that the client calls to reset a stopped audio stream. 
        /// Resetting the stream flushes all pending data and resets the audio clock stream 
        /// position to 0. This method fails if it is called on a stream that is not stopped
        /// </summary>
        public void Reset()
        {
            audioClientInterface.Reset();
        }

        #region IDisposable Members

        /// <summary>
        /// Dispose
        /// </summary>
        public void Dispose()
        {
            if (audioClientInterface != null)
            {
                if (audioRenderClient != null)
                {
                    audioRenderClient.Dispose();
                    audioRenderClient = null;
                }
                if (audioCaptureClient != null)
                {
                    audioCaptureClient.Dispose();
                    audioCaptureClient = null;
                }
                Marshal.ReleaseComObject(audioClientInterface);
                audioClientInterface = null;
                GC.SuppressFinalize(this);
            }
        }

        #endregion
    }
}
