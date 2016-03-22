using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace AudioIO_DLL.IO
{
    /// <summary>
    /// WaveFormatExtensible
    /// http://www.microsoft.com/whdc/device/audio/multichaud.mspx
    /// </summary>
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 2)]
    public struct WaveFormatExtensible
    {
        public static readonly Guid MEDIASUBTYPE_PCM = new Guid("00000001-0000-0010-8000-00AA00389B71"); // PCM audio. 
        public static readonly Guid MEDIASUBTYPE_IEEE_FLOAT = new Guid("00000003-0000-0010-8000-00aa00389b71"); // Corresponds to WAVE_FORMAT_IEEE_FLOAT 

        /// <summary>format type</summary>
        public WaveFormatEncoding WaveFormatTag;
        /// <summary>number of channels</summary>
        public short Channels;
        /// <summary>sample rate</summary>
        public int SampleRate;
        /// <summary>for buffer estimation</summary>
        public int AverageBytesPerSecond;
        /// <summary>block size of data</summary>
        public short BlockAlign;
        /// <summary>number of bits per sample of mono data</summary>
        public short BitsPerSample;
        /// <summary>number of following bytes</summary>
        public short ExtraSize;

        public short ValidBitsPerSample; // bits of precision, or is wSamplesPerBlock if wBitsPerSample==0
        public int ChannelMask; // which channels are present in stream
        public Guid SubFormat;

        public WaveFormatExtensible(int rate, int bits, int channels)
            : this()
        {
            if (channels < 1)
            {
                throw new ArgumentOutOfRangeException("Channels must be 1 or greater", "channels");
            }
            // minimum 16 bytes, sometimes 18 for PCM
            this.WaveFormatTag = WaveFormatEncoding.Pcm;
            this.Channels = (short)channels;
            this.SampleRate = rate;
            this.BitsPerSample = (short)bits;
            this.ExtraSize = 0;

            this.BlockAlign = (short)(channels * (bits / 8));
            this.AverageBytesPerSecond = this.SampleRate * this.BlockAlign;

            WaveFormatTag = WaveFormatEncoding.Extensible;
            ExtraSize = 22;
            ValidBitsPerSample = (short)bits;
            for (int n = 0; n < channels; n++)
            {
                ChannelMask |= (1 << n);
            }
            if (bits == 32)
            {
                // KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
                SubFormat = MEDIASUBTYPE_IEEE_FLOAT; // new Guid("00000003-0000-0010-8000-00aa00389b71");
            }
            else
            {
                // KSDATAFORMAT_SUBTYPE_PCM
                SubFormat = MEDIASUBTYPE_PCM; // new Guid("00000001-0000-0010-8000-00aa00389b71");
            }

        }

        public WaveFormatExtensible(WaveFormat waveFormat)
            : this()
        {
            this.WaveFormatTag = waveFormat.WaveFormatTag;
            this.Channels = waveFormat.Channels;
            this.SampleRate = waveFormat.SampleRate;
            this.AverageBytesPerSecond = waveFormat.AverageBytesPerSecond;
            this.BlockAlign = waveFormat.BlockAlign;
            this.BitsPerSample = waveFormat.BitsPerSample;
            this.ExtraSize = waveFormat.ExtraSize;
        }

        /// <summary>
        /// Helper function to retrieve a WaveFormat structure from a pointer
        /// </summary>
        /// <param name="pointer">WaveFormat structure</param>
        /// <returns></returns>
        public static WaveFormatExtensible MarshalFromPtr(IntPtr pointer)
        {
            //WaveFormat waveFormat = (WaveFormat)Marshal.PtrToStructure<WaveFormat>(pointer);
            WaveFormat waveFormat = (WaveFormat)Marshal.PtrToStructure(pointer, typeof(WaveFormat));
            WaveFormatExtensible waveFormatEx;
            switch (waveFormat.WaveFormatTag)
            {
                case WaveFormatEncoding.Pcm:
                    // can't rely on extra size even being there for PCM so blank it to avoid reading
                    // corrupt data
                    waveFormatEx = new WaveFormatExtensible(waveFormat);
                    break;
                case WaveFormatEncoding.Extensible:
                    waveFormatEx = (WaveFormatExtensible)Marshal.PtrToStructure(pointer, typeof(WaveFormatExtensible));
                    break;
                default:
                    waveFormatEx = new WaveFormatExtensible(waveFormat);

                    // Extra data not supported
                    waveFormatEx.ExtraSize = 0;
                    break;
            }

            return waveFormatEx;
        }

        /// <summary>
        /// Creates a WaveFormat with custom members
        /// </summary>
        /// <param name="tag">The encoding</param>
        /// <param name="sampleRate">Sample Rate</param>
        /// <param name="channels">Number of channels</param>
        /// <param name="averageBytesPerSecond">Average Bytes Per Second</param>
        /// <param name="blockAlign">Block Align</param>
        /// <param name="bitsPerSample">Bits Per Sample</param>
        /// <returns></returns>
        public static WaveFormatExtensible CreateCustomFormat(WaveFormatEncoding tag, int sampleRate, int channels, int averageBytesPerSecond, int blockAlign, int bitsPerSample)
        {
            WaveFormatExtensible waveFormat = new WaveFormatExtensible();
            waveFormat.WaveFormatTag = tag;
            waveFormat.Channels = (short)channels;
            waveFormat.SampleRate = sampleRate;
            waveFormat.AverageBytesPerSecond = averageBytesPerSecond;
            waveFormat.BlockAlign = (short)blockAlign;
            waveFormat.BitsPerSample = (short)bitsPerSample;
            waveFormat.ExtraSize = 0;
            return waveFormat;
        }

        /// <summary>
        /// Creates a new 32 bit IEEE floating point wave format
        /// </summary>
        /// <param name="sampleRate">sample rate</param>
        /// <param name="channels">number of channels</param>
        public static WaveFormatExtensible CreateIeeeFloatWaveFormat(int sampleRate, int channels)
        {
            WaveFormatExtensible wf = new WaveFormatExtensible();
            wf.WaveFormatTag = WaveFormatEncoding.IeeeFloat;
            wf.Channels = (short)channels;
            wf.BitsPerSample = 32;
            wf.SampleRate = sampleRate;
            wf.BlockAlign = (short)(4 * channels);
            wf.AverageBytesPerSecond = sampleRate * wf.BlockAlign;
            wf.ExtraSize = 0;
            return wf;
        }

        /// <summary>
        /// String representation
        /// </summary>
        public override string ToString()
        {
            return String.Format("{0} wBitsPerSample:{1} dwChannelMask:{2} subFormat:{3} extraSize:{4}",
                base.ToString(),
                ValidBitsPerSample,
                ChannelMask,
                SubFormat,
                ExtraSize);
        }
    }
}
