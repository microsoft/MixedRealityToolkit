using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace AudioIO_DLL.IO
{
    /// <summary>
    /// Represents a Wave file format
    /// </summary>
    [StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi, Pack=2)]
    public struct WaveFormat
    {
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

        /// <summary>
        /// Creates a new PCM format with the specified sample rate, bit depth and channels
        /// </summary>
        public WaveFormat(int rate, int bits, int channels)
        {
            if (channels < 1)
            {
                throw new ArgumentOutOfRangeException("Channels must be 1 or greater", "channels");
            }
            // minimum 16 bytes, sometimes 18 for PCM
            WaveFormatTag = WaveFormatEncoding.Pcm;
            Channels = (short)channels;
            SampleRate = rate;
            BitsPerSample = (short)bits;
            ExtraSize = 0;
                   
            BlockAlign = (short)(channels * (bits / 8));
            AverageBytesPerSecond = SampleRate * BlockAlign;
        }

        /// <summary>
        /// Reports this WaveFormat as a string
        /// </summary>
        /// <returns>String describing the wave format</returns>
        public override string ToString()
        {
            switch (this.WaveFormatTag)
            {
                case WaveFormatEncoding.Pcm:
                case WaveFormatEncoding.Extensible:
                    // extensible just has some extra bits after the PCM header
                    return String.Format("{0} bit PCM: {1}kHz {2} channels",
                        BitsPerSample, SampleRate / 1000, Channels);
                default:
                    return this.WaveFormatTag.ToString();
            }
        }
    }
}
