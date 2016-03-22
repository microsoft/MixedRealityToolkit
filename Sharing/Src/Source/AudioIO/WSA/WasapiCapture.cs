using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Windows.Devices.Enumeration;
using Windows.Media.Devices;
using Windows.ApplicationModel.Core;
using Windows.UI.Core;
using AudioIO_DLL.Utils;
using AudioIO_DLL.Core;
using AudioIO_DLL.Core.Interfaces;
using AudioIO_DLL.IO;


namespace AudioIO_DLL.IO
{
    /// <summary>
    /// Audio Capture using Wasapi
    /// See http://msdn.microsoft.com/en-us/library/dd370800%28VS.85%29.aspx for a generic description of 
    /// what this code is actually doing.
    /// </summary>
    public class WasapiCapture
    {
        private const long REFTIMES_PER_SEC = 10000000;
        private const long REFTIMES_PER_MILLISEC = 10000;
        private volatile bool pendingStopRequest;
        private byte[] recordBuffer;
        private string device;
        private bool isXMOS;
        private int bytesPerFrame;
        private WaveFormatExtensible waveFormat;
        private int frequency;
        private bool initialized;
        private AudioClient2 audioClient;

        /// <summary>
        /// Indicates recorded data is available 
        /// </summary>
        public event EventHandler<WaveInEventArgs> DataAvailableEventHandler;

        private int latencyMilliseconds;
        private EventWriterDLL EventWriterDLL = new EventWriterDLL();

        private static readonly uint Channel0 = 1 << 1; // Outside Left
        private static readonly uint Channel1 = 1 << 2; // Outside Right
        private static readonly uint Channel2 = 1 << 3; // Inside Left
        private static readonly uint Channel3 = 1 << 4; // Inside Right
        private static readonly uint Channel4 = 1 << 5; // Unused?
        private static readonly uint Channel5 = 1 << 6; // Unused?

        /// <summary>
        /// Initialises a new instance of the WASAPI capture class
        /// </summary>
        public WasapiCapture()
        {
            this.initialized = false;
            this.pendingStopRequest = false;
 
            this.isXMOS = false;
            this.EventWriterDLL.Initialize("AudioIO.IO::WasapiCapture");
        }

        /// <summary>
        /// Way of enumerating all the audio capture devices available on the system
        /// </summary>
        /// <returns></returns>
        private async Task<IEnumerable<DeviceInformation>> GetCaptureDevices()
        {
            string audioCaptureSelector = MediaDevice.GetAudioCaptureSelector();

            // (a PropertyKey)
            string supportsEventDrivenMode = "{1da5d803-d492-4edd-8c23-e0c0ffee7f0e} 7";
            IEnumerable<DeviceInformation> captureDevices = await DeviceInformation.FindAllAsync(audioCaptureSelector, new[] { supportsEventDrivenMode });

            return captureDevices;
        }

        /// <summary>
        /// Gets the default audio capture device
        /// </summary>
        /// <returns>The default audio capture device</returns>
        private async Task GetDefaultCaptureDevice()
        {
            // get the default device if available
            this.device = MediaDevice.GetDefaultAudioCaptureId(AudioDeviceRole.Default);
            IEnumerable<DeviceInformation> captureDevices = await GetCaptureDevices();

            if (this.device != null)
            {
                foreach (DeviceInformation di in captureDevices)
                {
                    if (di.Name.Contains("XMOS") == true && this.device.Equals(di.Id))
                    {
                        this.isXMOS = true;
                        this.EventWriterDLL.BuildLine("1a start => WasapiCapture::GetDefaultCaptureDevice => Using XMOS capture device: " + di.Name);
                        return;
                    }
                }
                this.EventWriterDLL.BuildLine("1b start => WasapiCapture::GetDefaultCaptureDevice => Using non-XMOS capture device.");
                return;
            }
            else
            {
                // If there is no defualt device, then pick the first device that is the XMOS device
                foreach (DeviceInformation di in captureDevices)
                {
                    if (di.Name.Contains("XMOS") == true)
                    {
                        this.device = di.Id;
                        this.isXMOS = true;
                        this.EventWriterDLL.BuildLine("1c start => WasapiCapture::GetDefaultCaptureDevice => Using capture device: " + di.Name);
                        return;
                    }
                }
            }
 
            this.EventWriterDLL.BuildLine("1d start fail => WasapiCapture::GetDefaultCaptureDevice =>NO APPROPRIATE MICROPHONE DEVICE FOUND - Check your sound settings!");
            this.device = null;
        }

        private async Task<IAudioClient2> Activate()
        {
            ActivateAudioInterfaceCompletionHandler activateCompletionHandler = new ActivateAudioInterfaceCompletionHandler(
                audioClient2 =>
                    {
                    this.InitializeCaptureDevice(audioClient2);
                    });
            Guid IID_IAudioClient2 = new Guid("726778CD-F60A-4eda-82DE-E47610CD78AA");
            IActivateAudioInterfaceAsyncOperation activationOperation;

            // This call must be made on the main UI thread.  
            NativeMethods.ActivateAudioInterfaceAsync(this.device, IID_IAudioClient2, IntPtr.Zero, activateCompletionHandler, out activationOperation);

            return await activateCompletionHandler.GetTask();
        }

        private void InitializeCaptureDevice(IAudioClient2 audioClientInterface)
        {
            this.audioClient = new AudioClient2(audioClientInterface);

            // Activation complete.  Set the client properties
            AudioClientProperties props = new AudioClientProperties();
            props.cbSize = 16; // (uint)System.Runtime.InteropServices.Marshal.SizeOf<AudioClientProperties>();
            props.bIsOffload = false;   // FALSE
            props.eCategory = AudioStreamCategory.Communications;  // AUDIO_STREAM_CATEGORY::AudioCategory_Communications in C++
            props.Options = AudioClientStreamOptions.None;              // AUDCLNT_STREAMOPTIONS_NONE in C++
            int hresult = audioClientInterface.SetClientProperties(props);
            if (hresult != 0)
            {
                Marshal.ThrowExceptionForHR(hresult);
            }

            this.waveFormat = this.audioClient.MixFormat;

            //if (this.isXMOS)
            //{
            //    // the mix format for the XMOS is likely a 6 channel interleaved audio stream that we don't need.
            //    // in theory, we should be able to just request a single channel 48K stream and it will just work 
            //    // and it will be good!!!  Of course, this doesn't actually work...  Set the veil audio mic to be 1 channel
            //    // before doing anything else... 

            //    this.waveFormat = WaveFormatExtensible.CreateIeeeFloatWaveFormat(48000, 1);
            //}


            long requestedDuration = REFTIMES_PER_MILLISEC * 100;
            this.frequency = 8 * waveFormat.AverageBytesPerSecond / (waveFormat.Channels * waveFormat.BitsPerSample);
            this.EventWriterDLL.BuildLine(
                "+2 start => WasapiCapture::InitializeCaptureDevice => Wave Format =\n" +
                    " => average bytes per second = " + waveFormat.AverageBytesPerSecond + "\n" +
                    " => bits per sample = " + waveFormat.BitsPerSample + "\n" +
                    " => channels = " + waveFormat.Channels + "\n" +
                    " => encoding = " + waveFormat.WaveFormatTag + "\n" +
                    " => extra size = " + waveFormat.ExtraSize + "\n" +
                    " => frequency = " + frequency);

            hresult = this.audioClient.Initialize(AudioClientShareMode.Shared,
                AudioClientStreamFlags.EventCallback,
                requestedDuration,
                0,
                ref this.waveFormat,
                Guid.Empty);

            if (hresult == 0)
            {
                int bufferFrameCount = this.audioClient.BufferSize;
                this.bytesPerFrame = this.waveFormat.Channels * this.waveFormat.BitsPerSample / 8;
                this.recordBuffer = new byte[bufferFrameCount * bytesPerFrame];
                this.EventWriterDLL.BuildLine(
                    "+3 => WasapiCapture::InitializeCaptureDevice => " + string.Format("record buffer size = {0}", this.recordBuffer.Length));

                // Get back the effective latency from AudioClient
                this.latencyMilliseconds = (int)(this.audioClient.StreamLatency / 10000);
                this.initialized = true;
            }
            else
            {
                this.EventWriterDLL.BuildLine("-3 => WasapiCapture::InitializeCaptureDevice => Error:" + string.Format("{0:X}", hresult));
                
            }
        }

        /// <summary>
        /// Start Recording
        /// </summary>
        public async void StartRecording()
        {
            try
            {
                IAudioClient2 audioClient = null;

                if (this.device == null)
                {
                    // no audio device is attached... so go get one if you can
                    await this.GetDefaultCaptureDevice();

                    if (this.device == null)
                    {
                        // OK, we tried and failed, so exit as gracefully as possible
                        return;
                    }
                }

                audioClient = await Activate();
                if (!this.initialized)
                {
                    return;     // couldn't initialize.
                }

                // force this to wait until the audioClient is done
                if (audioClient == null)
                {
                    this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "WasapiCapture::StartRecording => Could not activate audio client");
                    return;
                }

                var t = Task.Run(() => CaptureThread(audioClient));

                this.EventWriterDLL.BuildLine("+4 => WasapiCapture::StartRecording => StartRecording finished");
            } 
            catch (Exception e)
            {
                this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "WasapiCapture::StartRecording => Exception: " + e.ToString());
            }

            this.EventWriterDLL.FlushBuildString(EventWriterDLL.SeverityTypes.Information, 0x01);
        }

        /// <summary>
        /// Stop Recording
        /// </summary>
        public void StopRecording()
        {
            this.pendingStopRequest = true;
            // todo: wait for thread to end
            // todo: could signal the event
        }

        private void CaptureThread(IAudioClient2 audioClientIn)
        { 
            AudioClient2 audioClient = new AudioClient2(audioClientIn);
            Exception exception = null;
            try
            {
                DoRecording(audioClient);
            }
            catch (Exception e)
            {
                exception = e;
                this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "WasapiCapture::CaptureThread => Exception: " + e.ToString());
            }
            
        }

        /// <summary>
        /// This is the loop that does all of the hard work, each packet is recorded here and then dispatched.
        /// </summary>
        /// <param name="client"></param>
        private void DoRecording(AudioClient2 audioClient)
        {
            this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Information, 0x01,
                "WasapiCapture::DoRecording => has a buffer size of " + audioClient.BufferSize);

            int sampleBufferByteSize = audioClient.BufferSize * bytesPerFrame;
            byte[] sampleBuffer = new byte[sampleBufferByteSize];

            IntPtr audioSamplesReadyEventHandle = NativeMethods.CreateEventExW(IntPtr.Zero, IntPtr.Zero, 0, EventAccess.EVENT_ALL_ACCESS);
            audioClient.SetEventHandle(audioSamplesReadyEventHandle);

            try
            {
                 AudioCaptureClient captureClient = audioClient.AudioCaptureClient;
                audioClient.Start();

 
                while (!this.pendingStopRequest)
                {
                    NativeMethods.WaitForSingleObjectEx(audioSamplesReadyEventHandle, 1000, true);
                    int packetSize = captureClient.GetNextPacketSize();

                    if (packetSize > 0)
                    {
                        int numFramesToRead = 0;
                        int numBytesInSampleBuffer = 0;
                        AudioClientBufferFlags dwFlags = 0;
                        IntPtr micDataIn;

                        micDataIn = captureClient.GetBuffer(out numFramesToRead, out dwFlags);

                        int capturedBytes = numFramesToRead * bytesPerFrame;

                        if ((int)(dwFlags & AudioClientBufferFlags.Silent) > 0)
                        {
                            int maxBytes = Math.Min(capturedBytes, sampleBufferByteSize);
                            while (maxBytes-- > 0)
                            {
                                sampleBuffer[numBytesInSampleBuffer++] = 0;
                            }
                        }
                        else
                        {
                            System.Runtime.InteropServices.Marshal.Copy(micDataIn, sampleBuffer, 0, capturedBytes);
                            numBytesInSampleBuffer = capturedBytes;

                        }

                        captureClient.ReleaseBuffer(numFramesToRead);

                        if (DataAvailableEventHandler != null)
                        {
                            if (this.waveFormat.Channels == 2)
                            {
                                // convert stereo to mono inline!
                                ConvertStereoToMono(sampleBuffer, numBytesInSampleBuffer);
                                numBytesInSampleBuffer /= 2;
                            }
                            else if (this.waveFormat.Channels == 6)
                            {
                                // convert 6 to mono inline!
                                Convert6ToMono(sampleBuffer, numBytesInSampleBuffer, Channel2 | Channel3, 2);
                                numBytesInSampleBuffer /= 6;
                            }

                            DataAvailableEventHandler(this, new WaveInEventArgs(sampleBuffer, numBytesInSampleBuffer));
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                this.EventWriterDLL.WriteLine(EventWriterDLL.SeverityTypes.Error, 0x01, "WasapiCapture::DoRecording => Exception: " + ex.ToString());
            }
            finally
            {
                NativeMethods.CloseHandle(audioSamplesReadyEventHandle);
                audioClient.Stop();
                audioClient.Dispose();               
                this.pendingStopRequest = false;
            }
        }

        /// <summary>
        /// Convert a 6 channel input to mono out.  tweek the code below to pick just one channel or whatever...
        /// </summary>
        /// <param name="bytes"></param>
        /// <param name="size"></param>
        private static unsafe void Convert6ToMono(byte[] bytes, int size)
        {
            float floatValue1;
            float floatValue2;
            float floatValue3;
            float floatValue4;
            float floatValue5;
            float floatValue6;
            float floatValueMono;
            int numMonoFloats = size / 24; // 6 channels is 24 bytes

            // note, this averages the stereo pairs into the first half of the bytes array in mono!
            for (int i = 0; i < numMonoFloats; i++)
            {
                fixed (byte* bytePointer = &bytes[24 * i])
                {
                    floatValue1 = *(float*)bytePointer;
                }
                fixed (byte* bytePointer = &bytes[24 * i + 4])
                {
                    floatValue2 = *(float*)bytePointer;
                } 
                fixed (byte* bytePointer = &bytes[24 * i + 8])
                {
                    floatValue3 = *(float*)bytePointer;
                } 
                fixed (byte* bytePointer = &bytes[24 * i + 12])
                {
                    floatValue4 = *(float*)bytePointer;
                }
                fixed (byte* bytePointer = &bytes[24 * i + 16])
                {
                    floatValue5 = *(float*)bytePointer;
                } 
                fixed (byte* bytePointer = &bytes[24 * i + 20])
                {
                    floatValue6 = *(float*)bytePointer;
                }
                floatValueMono = (floatValue1 + floatValue2 + floatValue3 + floatValue4 + floatValue5 + floatValue6) / 6.0f;

                uint value = *((uint*)&floatValueMono);

                bytes[4 * i + 0] = (byte)(value & 0xFF);
                bytes[4 * i + 1] = (byte)((value >> 8) & 0xFF);
                bytes[4 * i + 2] = (byte)((value >> 16) & 0xFF);
                bytes[4 * i + 3] = (byte)((value >> 24) & 0xFF);
            }
        }



        private static unsafe void Convert6ToMono(byte[] bytes, int size, uint channelMask, int numChannelsToMerge)
        {
            int numMonoFloats = size / 24; // 6 channels is 24 bytes

            // note, this averages the stereo pairs into the first half of the bytes array in mono!
            for (int i = 0; i < numMonoFloats; i++)
            {
                float floatValue1 = 0f;
                float floatValue2 = 0f;
                float floatValue3 = 0f;
                float floatValue4 = 0f;
                float floatValue5 = 0f;
                float floatValue6 = 0f;

                if ((channelMask & Channel0) != 0)
                {
                    fixed (byte* bytePointer = &bytes[24 * i])
                    {
                        floatValue1 = *(float*)bytePointer;
                    }
                }

                if ((channelMask & Channel1) != 0)
                {
                    fixed (byte* bytePointer = &bytes[24 * i + 4])
                    {
                        floatValue2 = *(float*)bytePointer;
                    }
                }

                if ((channelMask & Channel2) != 0)
                {
                    fixed (byte* bytePointer = &bytes[24 * i + 8])
                    {
                        floatValue3 = *(float*)bytePointer;
                    }
                }

                if ((channelMask & Channel3) != 0)
                {
                    fixed (byte* bytePointer = &bytes[24 * i + 12])
                    {
                        floatValue4 = *(float*)bytePointer;
                    }
                }

                if ((channelMask & Channel4) != 0)
                {
                    fixed (byte* bytePointer = &bytes[24 * i + 16])
                    {
                        floatValue5 = *(float*)bytePointer;
                    }
                }

                if ((channelMask & Channel5) != 0)
                {
                    fixed (byte* bytePointer = &bytes[24 * i + 20])
                    {
                        floatValue6 = *(float*)bytePointer;
                    }
                }

                float floatValueMono = (floatValue1 + floatValue2 + floatValue3 + floatValue4 + floatValue5 + floatValue6) / (float)numChannelsToMerge;

                uint value = *((uint*)&floatValueMono);

                bytes[4 * i + 0] = (byte)(value & 0xFF);
                bytes[4 * i + 1] = (byte)((value >> 8) & 0xFF);
                bytes[4 * i + 2] = (byte)((value >> 16) & 0xFF);
                bytes[4 * i + 3] = (byte)((value >> 24) & 0xFF);
            }
        }


        private static unsafe void ConvertStereoToMono(byte[] bytes, int size)
        {
            float floatValueR;
            float floatValueL;
            float floatValueMono;
            int numMonoFloats = size / 8;

            // note, this averages the stereo pairs into the first half of the bytes array in mono!
            for (int i = 0; i < numMonoFloats; i++)
            {
                fixed (byte* bytePointer = &bytes[8 * i])
                {
                    floatValueR = *(float*)bytePointer;
                }
                fixed (byte* bytePointer = &bytes[8 * i + 4])
                {
                    floatValueL = *(float*)bytePointer;
                }
                floatValueMono = (floatValueR + floatValueL) / 2.0f;

                uint value = *((uint*)&floatValueMono);

                bytes[4 * i + 0] = (byte)(value & 0xFF);
                bytes[4 * i + 1] = (byte)((value >> 8) & 0xFF);
                bytes[4 * i + 2] = (byte)((value >> 16) & 0xFF);
                bytes[4 * i + 3] = (byte)((value >> 24) & 0xFF);
            }
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
