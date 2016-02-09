//#define TONEGENERATE

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using AudioIO_DLL.Core;
using AudioIO_DLL.Core.Interfaces;
using AudioIO_DLL.IO;
using Windows.Media.Devices;

using Windows.ApplicationModel.Core;
using Windows.UI.Core;
using AudioIO_DLL.Utils;
using Windows.Devices.Enumeration;

namespace AudioIO_DLL.IO
{
    /// <summary>
    /// WASAPI Out for Windows RT
    /// </summary>
    public class WasapiOut
    {
        private AudioClient2 audioClient;
        private string device;
        private readonly AudioClientShareMode shareMode;
        private AudioRenderClient audioRenderClient;
        private IWaveProvider sourceProvider;
        private int latencyMilliseconds;
        private int bufferFrameCount;
        private int bytesPerFrame;
        private byte[] readBuffer;
        private volatile PlaybackState playbackState;
        private WaveFormatExtensible renderWaveFormat;
        private IntPtr frameEventWaitHandle;
        private IntPtr initSync;
        private readonly SynchronizationContext syncContext;
        private int frequency;

        private EventWriterDLL EventWriterDLL = new EventWriterDLL();
        private bool waitingForInitToFinish = true;

        /// <summary>
        /// Playback Stopped
        /// </summary>
        public event EventHandler<StoppedEventArgs> PlaybackStopped;

        /// <summary>
        /// WasapiOut using default audio endpoint
        /// </summary>
        /// <param name="shareMode">ShareMode - shared or exclusive</param>
        /// <param name="latency">Desired latency in milliseconds</param>
        public WasapiOut(AudioClientShareMode shareMode, int latency)
        {
            this.EventWriterDLL.Initialize("AudioIO.IO::WasapiOut");
            this.shareMode = shareMode;
            this.latencyMilliseconds = latency;
            this.syncContext = SynchronizationContext.Current;
            frameEventWaitHandle = NativeMethods.CreateEventExW(IntPtr.Zero, IntPtr.Zero, 0, EventAccess.EVENT_ALL_ACCESS);
            initSync = NativeMethods.CreateEventExW(IntPtr.Zero, IntPtr.Zero, 0, EventAccess.EVENT_ALL_ACCESS);
        }

        private void InitializeAudio(IAudioClient2 audioClient2)
        {
                    long latencyRefTimes = latencyMilliseconds * 10000;
                    try
                    {
                this.audioClient = new AudioClient2(audioClient2);
                        this.renderWaveFormat = audioClient.MixFormat; // use the mix format by default

                        this.audioClient.Initialize(shareMode, AudioClientStreamFlags.EventCallback, latencyRefTimes, 0,
                                                   ref this.renderWaveFormat, Guid.Empty);

                        this.EventWriterDLL.BuildLine("+4 start => WasapiOutRT::Init2 => Initialized OK");
                    }
                    catch (Exception e)
                    {
                        this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "Exception in WasapiOutRT::Init2 trying to initialize audioClient: " + e.ToString());
                    }
        }

        /// <summary>
        /// Create, activate and initialize the AudioClient
        /// </summary>
        /// <returns></returns>
        private async Task Activate()
        {
            this.EventWriterDLL.BuildLine("+2 start => WasapiOutRT::Activate => starting");

            var activateCompletionHandler = new ActivateAudioInterfaceCompletionHandler(
                (audioClient2) =>
                {
                    this.InitializeAudio(audioClient2);
                });

            this.EventWriterDLL.BuildLine("+3 start => WasapiOutRT::Activate => getting default audio stream");
            
            var IID_IAudioClient2 = new Guid("726778CD-F60A-4eda-82DE-E47610CD78AA");
            IActivateAudioInterfaceAsyncOperation activationOperation;
            NativeMethods.ActivateAudioInterfaceAsync(this.device, IID_IAudioClient2, IntPtr.Zero, activateCompletionHandler, out activationOperation);

            await activateCompletionHandler.GetTask();

            this.EventWriterDLL.BuildLine("+5 start => WasapiOutRT::Activate => creating AudioClient");
        }

        private async Task GetDefaultAudioEndpoint()
        {
            // get the default device if it is there!
            this.device = MediaDevice.GetDefaultAudioRenderId(AudioDeviceRole.Default);
            if (this.device != null)
            {
                this.EventWriterDLL.BuildLine("+1a start => WasapiOutRT::GetDefaultAudioEndpoint => GetDefaultAudioRenderId found: " + this.device);
                return;
            }

            // if there is no default or it is disconnected, get the XMOS device specifically!!!
            string audioRenderSelector = MediaDevice.GetAudioRenderSelector();
            var playbackDevices = await DeviceInformation.FindAllAsync(audioRenderSelector);

            foreach (DeviceInformation di in playbackDevices)
            {
                this.EventWriterDLL.BuildLine("+1a start => WasapiOutRT::GetDefaultAudioEndpoint => Playback device found: " + di.Name);

                if (di.Name.Contains("XMOS") == true)
                {
                    this.device = di.Id;
                    NativeMethods.SetEvent(initSync);
                    this.EventWriterDLL.BuildLine("+1b start => WasapiOutRT::GetDefaultAudioEndpoint => Using playback device: " + di.Name);

                    return;
                }

                //if (di.Name.Contains("XMOS") != true && di.Name.Contains("Digital Audio") != true)
                //{
                //    this.device = di.Id;
                //    NativeMethods.SetEvent(initSync);
                //    this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Information, 0x01, "+1b start => WasapiOutRT::GetDefaultAudioEndpoint => Using playback device: " + di.Name);

                //    return;
                //}
            }

            this.EventWriterDLL.BuildLine("+1c start => WasapiOutRT::GetDefaultAudioEndpoint => NO APPROPRIATE SPEAKER DEVICE FOUND - Check your sound settings!");
            NativeMethods.SetEvent(initSync);
            this.device = null;
        }

        /// <summary>
        /// This is the thread that does the hard work.  Packets are recieved and this thread gets the data and passes it to the
        /// output channel correctly.
        /// </summary>
        private async void PlayThread()
        {
            Exception exception = null;

            try
            {
                // make sure the initializer has a chance to finish the init process.
                this.EventWriterDLL.BuildLine("+8 start => WasapiOutRT::PlayThread => before while loop, waitingForInitToFinish is: " + this.waitingForInitToFinish);
                while (this.waitingForInitToFinish)
                {
                    await Task.Delay(2);
                }

                this.EventWriterDLL.BuildLine("+9 start => WasapiOutRT::PlayThread => after while loop, waitingForInitToFinish is: " + this.waitingForInitToFinish);
                this.audioClient.SetEventHandle(frameEventWaitHandle);

                this.frequency = 8 * this.renderWaveFormat.AverageBytesPerSecond / (this.renderWaveFormat.Channels * this.renderWaveFormat.BitsPerSample);
                this.EventWriterDLL.BuildLine(
                    "+10 start =>WasapiOutRT::PlayThread => Wave Format =\n" +
                        " => average bytes per second = " + this.renderWaveFormat.AverageBytesPerSecond + "\n" +
                        " => bits per sample = " + this.renderWaveFormat.BitsPerSample + "\n" +
                        " => channels = " + this.renderWaveFormat.Channels + "\n" +
                        " => encoding = " + this.renderWaveFormat.WaveFormatTag + "\n" +
                        " => extra size = " + this.renderWaveFormat.ExtraSize + "\n" +
                        " => frequency = " + frequency);

                // fill a whole buffer
                this.bufferFrameCount = this.audioClient.BufferSize; // this calls GetBufferSize under the surface!
                this.bytesPerFrame = this.renderWaveFormat.Channels * this.renderWaveFormat.BitsPerSample / 8;
                this.readBuffer = new byte[this.bufferFrameCount * this.bytesPerFrame];

                IWaveProvider playbackProvider = this.sourceProvider;
     
                this.audioClient.Start();
                this.EventWriterDLL.BuildLine("+11 start => WasapiOutRT::PlayThread => audio client started and looping");

                // Get back the effective latency from AudioClient
                latencyMilliseconds = (int)(this.audioClient.StreamLatency / 10000);

                while (this.playbackState != PlaybackState.Stopped)
                {
                    await Task.Delay(10);
                    int numFramesAvailable = this.bufferFrameCount - this.audioClient.CurrentPadding;
                    FillBuffer(playbackProvider, numFramesAvailable);
/***************************
                    // If using Event Sync, Wait for notification from AudioClient or Sleep half latency
                    int timeout = 3 * latencyMilliseconds;
                    var r = NativeMethods.WaitForSingleObjectEx(frameEventWaitHandle, timeout, true);
                    if (r != 0)
                    {
                        this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Warning, 0x01, "WasapiOutRT::PlayThread => Timed out waiting for event in WasapiOut");
                    }
                    else
                    {
                        // If still playing and notification is ok
                        if (this.playbackState == PlaybackState.Playing)
                        {
                            int numFramesAvailable = this.bufferFrameCount - this.audioClient.CurrentPadding;
                            FillBuffer(playbackProvider, numFramesAvailable);
                        }
                    }
 ******************************/

                 }
                // play the buffer out
                while (this.audioClient.CurrentPadding > 0)
                {
                    await Task.Delay(latencyMilliseconds / 2);
                }
                this.audioClient.Stop();
                if (this.playbackState == PlaybackState.Stopped)
                {
                    this.audioClient.Reset();
                }
            }
            catch (Exception e)
            {
                exception = e;
                this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "WasapiOutRT::PlayThread => Exception: " + e.ToString());
            }
            finally
            {
                RaisePlaybackStopped(exception);
            }
        }

        private void RaisePlaybackStopped(Exception e)
        {
            var handler = PlaybackStopped;
            if (handler != null)
            {
                if (this.syncContext == null)
                {
                    handler(this, new StoppedEventArgs(e));
                }
                else
                {
                    syncContext.Post(state => handler(this, new StoppedEventArgs(e)), null);
                }
            }
        }

        /// <summary>
        /// Fill a properly marshalled buffer with audio data.  It looks simple enough, but
        /// this is often the source of a lot of playback issues.
        /// </summary>
        /// <param name="playbackProvider"></param>
        /// <param name="frameCount"></param>
        /// <returns></returns>
        private int FillBuffer(IWaveProvider playbackProvider, int maxFramesToWrite)
        {
            int actualByteCount;
            int maxBytesToWrite = maxFramesToWrite * this.bytesPerFrame;

#if TONEGENERATE
            actualByteCount = GenerateTone(this.readBuffer, 0, maxBytesToWrite);
#else
            actualByteCount = playbackProvider.Read(this.readBuffer, 0, maxBytesToWrite);
#endif
            int actualFrameCount = actualByteCount / this.bytesPerFrame;

 
            if (actualByteCount > 0)
            {
                IntPtr renderClientOutBuffer = this.audioRenderClient.GetBuffer(actualFrameCount);
                Marshal.Copy(this.readBuffer, 0, renderClientOutBuffer, actualByteCount);

                this.audioRenderClient.ReleaseBuffer(actualFrameCount, AudioClientBufferFlags.None);
            }

            return actualFrameCount;
        }

        private static unsafe int GenerateTone(byte[] buffer, int offset, int length)
        {
            double sampleRate = 48000f;
            float flt;
            uint value;
            for (int n = 0; n < length / 4; n++)
            {
                flt = (float)(0.1 * Math.Sin((2 * Math.PI * n * 600.0) / sampleRate));
                value = *((uint*)&flt);

                buffer[4 * n + 0] = (byte) (value           & 0xFF);
                buffer[4 * n + 1] = (byte)((value >> 8)     & 0xFF);
                buffer[4 * n + 2] = (byte)((value >> 16)    & 0xFF);
                buffer[4 * n + 3] = (byte)((value >> 24)    & 0xFF);
            }
            return length;
        }

        #region IWavePlayer Members

        /// <summary>
        /// Begin Playback
        /// </summary>
        async public void Play()
        {
            if (this.playbackState != PlaybackState.Playing)
            {
                if (this.playbackState == PlaybackState.Stopped)
                {
                    this.EventWriterDLL.FlushBuildString(EventWriterDLL.SeverityTypes.Information, 0x01);

                    this.playbackState = PlaybackState.Playing;
                    await Task.Delay(10);
                    await Task.Run(() => PlayThread());
                }
                else
                {
                    this.playbackState = PlaybackState.Playing;
                }
            }
        }

        /// <summary>
        /// Stop playback and flush buffers
        /// </summary>
        public void Stop()
        {
            if (this.playbackState != PlaybackState.Stopped)
            {
                this.playbackState = PlaybackState.Stopped;
            }
        }

        /// <summary>
        /// Stop playback without flushing buffers
        /// </summary>
        public void Pause()
        {
            if (this.playbackState == PlaybackState.Playing)
            {
                this.playbackState = PlaybackState.Paused;
            }
        }

        /// <summary>
        /// Init the output audio device
        /// </summary>
        /// <param name="waveProvider">waveProvider interface</param>
        public async Task Init(IWaveProvider waveProvider)
        {
            if (this.device == null)
            {
                // try our hardest to get a divice if the string is null or empty
                await GetDefaultAudioEndpoint();
                NativeMethods.WaitForSingleObjectEx(initSync, 1000, true);
            }

            if (this.device != null)
            {
                try
                {
                    await CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(CoreDispatcherPriority.Normal,
                        async () =>
                        {
                            try
                            {
                            this.EventWriterDLL.BuildLine("+1 start => WasapiOutRT::Init2 => called");

                            this.renderWaveFormat = waveProvider.WaveFormat;
                            await Activate();

                            this.EventWriterDLL.BuildLine("+8 start => WasapiOutRT::Init2 => resampler not needed");
                            this.sourceProvider = waveProvider;

                            // Get the RenderClient
                            this.audioRenderClient = audioClient.AudioRenderClient;

                            this.waitingForInitToFinish = false;

                            this.EventWriterDLL.BuildLine("+9 start => WasapiOutRT::Init2 => finished");
                            }
                            catch (Exception ex)
                            {
                                this.EventWriterDLL.WriteLine(Utils.EventWriterDLL.SeverityTypes.Error, 0x01, "WasapiOutRT::Init2 => Exception: " + ex.Message);
                            }
                        });
                }
                catch (Exception ex)
                {
                    Debug.WriteLine("Exception in WasapiOutRT, waveProvider = " + waveProvider.ToString() + ", " + ex.Message);
                    this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "WasapiOutRT::Init2 => waveProvider = " + waveProvider.ToString() + ", exception: " + ex.Message);
                }
            }
        }

        /// <summary>
        /// Playback State
        /// </summary>
        public PlaybackState PlaybackState
        {
            get { return this.playbackState; }
        }

        #endregion

        #region IDisposable Members

        /// <summary>
        /// Dispose
        /// </summary>
        public void Dispose()
        {
            if (audioClient != null)
            {
                Stop();

                this.audioClient.Dispose();
                this.audioClient = null;
                this.audioRenderClient = null;
                NativeMethods.CloseHandle(frameEventWaitHandle);
            }

        }

        #endregion
    }

    /// <summary>
    /// Come useful native methods for Windows 8.1 Metro support
    /// </summary>
    class NativeMethods
    {
        [DllImport("api-ms-win-core-synch-l1-1-0.dll", CharSet = CharSet.Unicode, ExactSpelling = false, PreserveSig = true, SetLastError = true)]
        internal static extern IntPtr CreateEventExW(IntPtr lpEventAttributes, IntPtr lpName, int dwFlags, EventAccess dwDesiredAccess);

        [DllImport("api-ms-win-core-synch-l1-1-0.dll", CharSet = CharSet.Unicode, ExactSpelling = false, PreserveSig = true, SetLastError = true)]
        internal static extern bool SetEvent(IntPtr hEvent);

        [DllImport("api-ms-win-core-handle-l1-1-0.dll", ExactSpelling = true, PreserveSig = true, SetLastError = true)]
        public static extern bool CloseHandle(IntPtr hObject);

        [DllImport("api-ms-win-core-synch-l1-1-0.dll", ExactSpelling = true, PreserveSig = true, SetLastError = true)]
        public static extern int WaitForSingleObjectEx(IntPtr hEvent, int milliseconds, bool bAlertable);

        /// <summary>
        /// Enables Windows Store apps to access preexisting Component Object Model (COM) interfaces in the WASAPI family.
        /// </summary>
        /// <param name="deviceInterfacePath">A device interface ID for an audio device. This is normally retrieved from a DeviceInformation object or one of the methods of the MediaDevice class.</param>
        /// <param name="riid">The IID of a COM interface in the WASAPI family, such as IAudioClient.</param>
        /// <param name="activationParams">Interface-specific activation parameters. For more information, see the pActivationParams parameter in IMMDevice::Activate. </param>
        /// <param name="completionHandler"></param>
        /// <param name="activationOperation"></param>
        [DllImport("Mmdevapi.dll", ExactSpelling = true, PreserveSig = false)]
        public static extern void ActivateAudioInterfaceAsync(
            [In, MarshalAs(UnmanagedType.LPWStr)] string deviceInterfacePath,
            [In, MarshalAs(UnmanagedType.LPStruct)] Guid riid,
            [In] IntPtr activationParams, // n.b. is actually a pointer to a PropVariant, but we never need to pass anything but null
            [In] IActivateAudioInterfaceCompletionHandler completionHandler,
            out IActivateAudioInterfaceAsyncOperation activationOperation);
    }

    [Flags]
    internal enum EventAccess
    {
        STANDARD_RIGHTS_REQUIRED = 0xF0000,
        SYNCHRONIZE = 0x100000,
        EVENT_ALL_ACCESS = STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x3
    }

   
}
