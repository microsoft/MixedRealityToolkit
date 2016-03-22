using System;

namespace AudioIO_DLL.IO
{
    /// <summary>
    /// Like IWaveProvider, but makes it much simpler to put together a 32 bit floating point mixing engine
    /// </summary>
    public interface ISampleProvider
    {
        /// <summary>
        /// Gets the WaveFormat of this Sample Provider.
        /// </summary>
        WaveFormatExtensible WaveFormat { get; }

        /// <summary>
        /// Fill the specified buffer with 32 bit floating point samples
        /// </summary>
        /// <param name="buffer">The buffer to fill with samples</param>
        /// <param name="offset">Offset into buffer</param>
        /// <param name="count">The number of samples to read</param>
        /// <returns></returns>
        int Read(float[] buffer, int offset, int count);
    }
}