using AudioIO_DLL.IO;
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace AudioIO_DLL.Utils
{
    /// <summary>
    /// A very basic circular buffer implementation that also implements 
    /// IWaveProvider and ISampleProvider.
    /// </summary>
    public class HRTFBufferDLL : IWaveProvider, ISampleProvider
    {
        private WaveFormatExtensible waveFormat;

        private float[] bufferRight;
        private float[] bufferLeft;
        
        // these temp arrays are passed into the HRTF processor
        private readonly object lockObjectHRTF;

        // due to a bug in the WaveBuffer class, this needs to be 4 times bigger because
        // floats are 4 times bigger than bytes.  Not sure how to fix this as it looks like it
        // is part of the C# definition!!!
        private float[] theHRTFProcessBuffer = new float[4 * BufferConstants.HRTFBufferDLLSize];

        // TODO: these need to be longer circular buffers so that audio can be accumulated corrrectly
        private float[] theHRTFProcessBufferR = new float[BufferConstants.HRTFBufferDLLSize];
        private float[] theHRTFProcessBufferL = new float[BufferConstants.HRTFBufferDLLSize];

        private readonly object lockObjectRL;
        private int writePositionR; 
        private int writePositionL;
        private int readPosition;
        private int floatCount;

        //---------------------------------------------------------------------
        // HRTF information

        /// <summary>
        /// bool that turns on/off the HRTF processing.  If off, the audio is copied equally 
        /// to the right and left speakers.
        /// </summary>
        public bool hrtfOn { get; set; }

        /// <summary>
        /// HRTF X relative position
        /// </summary>
        public float atX { get; set; }

        /// <summary>
        /// HRTF Y relative position
        /// </summary>
        public float atY { get; set; }

        /// <summary>
        /// HRTF Z relative position
        /// </summary>
        public float atZ { get; set; }

        /// <summary>
        /// HRTF gain for this object
        /// </summary>
        public float audioGain { get; set; }

        private const float eps = 0.00000001f;

        //--------------------------------------------------------------------
        /// <summary>
        /// Create a new HRTFBufferDLL
        /// </summary>
        /// <param name="size">Max buffer size in bytes</param>
        public HRTFBufferDLL(int size)
            : this(BufferConstants.BaseFrequency, BufferConstants.AudioChannelsAfterHRTF)
        {
            this.audioGain = 1.0f;
            this.bufferRight = new float[size];
            this.bufferLeft = new float[size];
        }

        /// <summary>
        /// Initializes a new instance of the HRTFBufferDLL class 
        /// defaulting to 44.1kHz mono
        /// </summary>
        public HRTFBufferDLL()
            : this(BufferConstants.BaseFrequency, BufferConstants.AudioChannelsAfterHRTF)
        {
        }

        /// <summary>
        /// Initializes a new instance of the HRTFBufferDLL class with the specified
        /// sample rate and number of channels
        /// </summary>
        public HRTFBufferDLL(int sampleRate, int channels)
        {
            this.bufferRight = new float[BufferConstants.QuarterSecond];
            this.bufferLeft = new float[BufferConstants.QuarterSecond];
            this.lockObjectHRTF = new object();
            this.lockObjectRL = new object();

            SetWaveFormat(sampleRate, channels);

            //HRTFBufferDLL.Initialize(); // init the HRTF engine
        }

        /// <summary>
        /// Allows you to specify the sample rate and channels for this WaveProvider
        /// (should be initialised before you pass it to a wave player)
        /// </summary>
        public void SetWaveFormat(int sampleRate, int channels)
        {
            this.waveFormat = WaveFormatExtensible.CreateIeeeFloatWaveFormat(sampleRate, channels);
        }

        /// <summary>
        /// The Wave Format
        /// </summary>
        public WaveFormatExtensible WaveFormat
        {
            get { return waveFormat; }
        }

        /// <summary>
        /// An accumulator of type CircularBufferDLL can accumulate incoming packets and 
        /// when enough data points are presnet, we copy it over and HRTF process.
        /// </summary>
        /// <param name="cb"></param>
        public void ReadFromAccumulator(CircularBuffer cb)
        {
            lock (lockObjectHRTF)
            {
                cb.ReadFloats(this.theHRTFProcessBuffer, 0, BufferConstants.HRTFBufferDLLSize);
            }
        }

        /// <summary>
        /// Processes the mono input stream to HRTF stereo output stream.
        /// </summary>
        public void ProcessHRTF()
        {
            lock (this.lockObjectHRTF)
            {
                //if (this.hrtfOn)
                //{
                //    // do the HRTF processing
                //    bool f = HRTFBufferDLL.ProcessAtPosition(this.atX, this.atY, this.atZ, this.theHRTFProcessBuffer, this.theHRTFProcessBufferL, this.theHRTFProcessBufferR);

                //    if (f)
                //    {
                //        float outRMSR = 0.0f;
                //        float outRMSL = 0.0f;
                //        // Check to see if the input intensities and output intensities are about the same...
                //        for (int i = 0; i < HRTFBufferDLL.HRTFBufferDLLSize; i += 10) // don't need to do every point, every 10th works fine
                //        {
                //            outRMSR += this.theHRTFProcessBufferR[i] * this.theHRTFProcessBufferR[i];
                //            outRMSL += this.theHRTFProcessBufferL[i] * this.theHRTFProcessBufferL[i];
                //        }

                //        if (outRMSR < eps || outRMSL < eps)
                //        {
                //            f = false; // something went wrong...
                //        }
                //    }

                //    if (!f)
                //    {
                //        for (int i = 0; i < HRTFBufferDLL.HRTFBufferDLLSize; i++)
                //        {
                //            this.theHRTFProcessBufferL[i] = this.theHRTFProcessBuffer[i];
                //            this.theHRTFProcessBufferR[i] = this.theHRTFProcessBuffer[i];
                //        }
                //    }
                //}
                //else
                {
                    // no HRTF, just copy mono to each channel
                    for (int i = 0; i < BufferConstants.HRTFBufferDLLSize; i++)
                    {
                        this.theHRTFProcessBufferL[i] = this.theHRTFProcessBuffer[i];
                        this.theHRTFProcessBufferR[i] = this.theHRTFProcessBuffer[i];
                    }
                }

                // in either case, we write the processed data to the R and L buffers, note
                // that both the HRTF and R/L buffers are locked for this copy!
                lock (this.lockObjectRL)
                {
                    this.Write(this.bufferLeft, this.theHRTFProcessBufferL, ref this.writePositionL);
                    this.floatCount += this.Write(this.bufferRight, this.theHRTFProcessBufferR, ref this.writePositionR);
                }
            }
        }

        private int Write(float[] buffer, float[] HRTFBufferDLL, ref int writePosition)
        {
            // lock is done by caller

            var bytesWritten = 0;
            int count = BufferConstants.HRTFBufferDLLSize;
            if (count > buffer.Length - this.floatCount)
            {
                count = buffer.Length - this.floatCount;
            }
            // write to end
            int writeToEnd = Math.Min(buffer.Length - writePosition, count);
            Array.Copy(HRTFBufferDLL, 0, buffer, writePosition, writeToEnd);
            writePosition += writeToEnd;
            writePosition %= buffer.Length;
            bytesWritten += writeToEnd;
            if (bytesWritten < count)
            {
                Debug.Assert(writePosition == 0);
                // must have wrapped round. Write to start
                Array.Copy(HRTFBufferDLL, bytesWritten, buffer, writePosition, count - bytesWritten);
                writePosition += (count - bytesWritten);
                bytesWritten = count;
            }

            return bytesWritten;
        }

        /// <summary>
        /// Method to override in derived classes
        /// Supply the requested number of samples into the buffer
        /// </summary>
        public int Read(float[] buffer, int offset, int sampleCount)
        {
            // we need to step through the actual buffer and interleave the floats from right and left
            lock (this.lockObjectRL)
            {
                int halfSampleCount = sampleCount / 2;
                int sampleCountToProcess = Math.Min(halfSampleCount, this.floatCount);
                int remainingCount = sampleCountToProcess - halfSampleCount;

                this.floatCount -= sampleCountToProcess;
                while (sampleCountToProcess-- > 0)
                {
                    buffer[offset++] = this.bufferLeft[this.readPosition];
                    buffer[offset++] = this.bufferRight[this.readPosition];
                    if (++this.readPosition == bufferRight.Length)
                    {
                        this.readPosition = 0;
                    }
                }

                while (remainingCount-- > 0)
                {
                    float f = (float)(0.02 * Math.Sin((2.0 * Math.PI * (float)remainingCount * 1800.0) / 48000.0));
                    buffer[offset++] = f;
                    buffer[offset++] = f;
                }
            }

            return sampleCount;
        }


        /// <summary>
        /// Read from the buffer
        /// </summary>
        /// <param name="data">Buffer to read into</param>
        /// <param name="offset">Offset into read buffer</param>
        /// <param name="count">Bytes to read</param>
        /// <returns>Number of bytes actually read</returns>
        //public int Read(byte[] data, int offset, int count)
        //{
        //    //WaveBuffer waveBuffer = new WaveBuffer(data);
        //    //int samplesRead = Read(waveBuffer.FloatBuffer, offset / 4, count / 4);
        //    //return samplesRead * 4;

        //    // we need to step through the actual buffer and interleave the floats from right and left
        //    lock (this.lockObjectRL)
        //    {
        //        int sampleCount = count / 4;

        //        int halfSampleCount = sampleCount / 2;
        //        int sampleCountToProcess = Math.Min(halfSampleCount, this.floatCount);
        //        int remainingCount = sampleCountToProcess - halfSampleCount;

        //        this.floatCount -= sampleCountToProcess;
        //        byte[] btR = new byte[4];
        //        byte[] btL = new byte[4];
        //        while (sampleCountToProcess-- > 0)
        //        {
        //            btR = BitConverter.GetBytes(this.bufferLeft[this.readPosition]);
        //            data[offset++] = btR[0];
        //            data[offset++] = btR[1];
        //            data[offset++] = btR[2];
        //            data[offset++] = btR[3];
        //            btL = BitConverter.GetBytes(this.bufferRight[this.readPosition]);
        //            data[offset++] = btL[0];
        //            data[offset++] = btL[1];
        //            data[offset++] = btL[2];
        //            data[offset++] = btL[3];

        //            if (++this.readPosition == this.bufferLeft.Length)
        //            {
        //                this.readPosition = 0;
        //            }
        //        }

        //        while (remainingCount-- > 0)
        //        {
        //            float f = (float)(0.02 * Math.Sin((2.0 * Math.PI * (float)remainingCount * 1800.0) / 48000.0));
        //            btL = BitConverter.GetBytes(f);
        //            data[offset++] = btL[0];
        //            data[offset++] = btL[1];
        //            data[offset++] = btL[2];
        //            data[offset++] = btL[3]; 
                    
        //            data[offset++] = btL[0];
        //            data[offset++] = btL[1];
        //            data[offset++] = btL[2];
        //            data[offset++] = btL[3];
                    
        //        }
        //    }

        //    return count;
        //}

        public int Read(byte[] byteBuffer, int byteOffset, int byteCount)
        {
            // we need to step through the actual buffer and interleave the floats from right and left
            lock (this.lockObjectRL)
            {
                int sampleCount = byteCount / 4;  // 4 bytes per float

                int halfSampleCount = sampleCount / 2;
                int sampleCountToProcess = Math.Min(halfSampleCount, this.floatCount);
                int remainingCount = sampleCountToProcess - halfSampleCount;
                int actualByteCount = sampleCountToProcess * 8; // 2 channels per sample * 4 bytes per float
                this.floatCount -= sampleCountToProcess;
                while (sampleCountToProcess-- > 0)
                {
                    ConvertFloatToBytes(this.bufferLeft[this.readPosition], byteBuffer, byteOffset);
                    byteOffset += 4;
                    ConvertFloatToBytes(this.bufferRight[this.readPosition], byteBuffer, byteOffset);
                    byteOffset += 4;

                    if (++this.readPosition == this.bufferLeft.Length)
                    {
                        this.readPosition = 0;
                    }
                }
                /*******************
                while (remainingCount-- > 0)
                {
                    float f = (float)(0.02 * Math.Sin((2.0 * Math.PI * (float)remainingCount * 1800.0) / 48000.0));
                    ConvertFloatToBytes(f, byteBuffer, byteOffset);
                    byteOffset += 4;
                    ConvertFloatToBytes(f, byteBuffer, byteOffset);
                    byteOffset += 4;
                }
                *******************/
                return actualByteCount;
            }
        }

        public static unsafe void ConvertFloatToBytes(float floatValue, byte[] outBytes, int offset)
        {
            uint value = *((uint*)&floatValue);
            outBytes[offset + 0] = (byte)(value & 0xFF);
            outBytes[offset + 1] = (byte)((value >> 8) & 0xFF);
            outBytes[offset + 2] = (byte)((value >> 16) & 0xFF);
            outBytes[offset + 3] = (byte)((value >> 24) & 0xFF);
        }

        /// <summary>
        /// Maximum length of this circular buffer
        /// </summary>
        public int MaxLength
        {
            get { return this.bufferRight.Length; }
        }

        /// <summary>
        /// Number of bytes currently stored in the circular buffer
        /// </summary>
        public int Count
        {
            get { return this.floatCount; }
        }

        /// <summary>
        /// Resets the buffer
        /// </summary>
        public void Reset()
        {
            this.floatCount = 0;
            this.readPosition = 0;
            this.writePositionR = 0;
            this.writePositionL = 0;

            // we need to clear the buffer just in case another thread reads
            // the volume after Stopping!
            Array.Clear(this.bufferRight, 0, this.bufferRight.Length);
            Array.Clear(this.bufferLeft, 0, this.bufferLeft.Length);
        }

        /// <summary>
        /// Advances the buffer, discarding bytes
        /// </summary>
        /// <param name="count">Bytes to advance</param>
        public void Advance(int count)
        {
            if (count >= floatCount)
            {
                Reset();
            }
            else
            {
                floatCount -= count;
                readPosition += count;
                readPosition %= MaxLength;
            }
        }

        //----------------------------------------------------------------------------------------------------
        // extern HRTF calls

        //[DllImport("SOTAHrtf", CallingConvention = CallingConvention.Cdecl)]
        //public static extern int Initialize();

        //[DllImport("SOTAHrtf", CallingConvention = CallingConvention.Cdecl)]
        //public static extern void Uninitialize();

        //[DllImport("SOTAHrtf", CallingConvention = CallingConvention.Cdecl)]
        //public static extern void SetPlane(int pIndex, float a1, float a2, float a3);

        //[DllImport("SOTAHrtf", CallingConvention = CallingConvention.Cdecl)]
        //public static extern void SetSourceProp(int index, int sourceProp, float value);

        //[DllImport("SOTAHrtf", CallingConvention = CallingConvention.Cdecl)]
        //public static extern void SetDirectivity(float azimuth, bool setDirectivity);

        //[DllImport("SOTAHrtf", CallingConvention = CallingConvention.Cdecl)]
        //public static extern void SetHRTFParams(float radius, float direct, float reflect, float reverb);

        //[DllImport("SOTAHrtf", CallingConvention = CallingConvention.Cdecl)]
        //public static extern bool ProcessAtPosition(float atX, float atY, float atZ, float[] pInBuffer, float[] pOutBufferL, float[] pOutBufferR);
    }
}

