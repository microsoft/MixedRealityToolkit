using AudioIO_DLL.IO;
using AudioIO_DLL.Utils;
using System;
using System.Diagnostics;

namespace AudioIO_DLL.Utils
{
    /// <summary>
    /// Circular Buffer that is intended to hold floats, but also allows
    /// read/write of those floats as byte arrays.
    /// </summary>
    public class CircularBuffer
    {
        private readonly byte[] buffer;
        private readonly object lockObject;
        private int writePosition;
        private int readPosition;
        private int numBytesInBuffer;

        /// <summary>
        /// Create a new circular buffer
        /// </summary>
        /// <param name="numFloats">Max buffer size in floats</param>
        public CircularBuffer(int numFloats)
        {
            this.buffer = new byte[4 * numFloats]; // convert float count to byte count
            this.lockObject = new object();
            Reset();
        }


        /// <summary>
        /// Read bytes out of the buffer.
        /// </summary>
        /// <param name="outByteData">Buffer to accept bytes read</param>
        /// <param name="outByteOffset">Start offset in outByteData</param>
        /// <param name="readByteCount"># of bytes to read</param>
        /// <returns>Number of bytes actually read</returns>
        public int ReadBytes(byte[] outByteData, int outByteOffset, int readByteCount)
        {
            lock (lockObject)
            {
                readByteCount = Math.Min(readByteCount, outByteData.Length - outByteOffset);
                readByteCount = Math.Min(readByteCount, this.numBytesInBuffer);
                
                int count = readByteCount;
                while (count-- > 0)
                {
                    outByteData[outByteOffset++] = this.buffer[this.readPosition++];
                    if (this.readPosition == this.buffer.Length)
                    {
                        this.readPosition = 0;
                    }
                }
                this.numBytesInBuffer -= readByteCount;
                return readByteCount;
            }
        }


        /// <summary>
        /// Read floats out of the buffer.
        /// </summary>
        /// <param name="outFloatData">Buffer to accept floats read</param>
        /// <param name="outFloatOffset">Start offset in outFloatData</param>
        /// <param name="readFloatCount"># of floats to read</param>
        public int ReadFloats(float[] outFloatData, int outFloatOffset, int readFloatCount)
        {
            lock (lockObject)
            {
                Debug.Assert((this.readPosition & 3) == 0);
  
                // do all of our counting here in floats
                readFloatCount = Math.Min(readFloatCount, outFloatData.Length - outFloatOffset);
                readFloatCount = Math.Min(readFloatCount, this.numBytesInBuffer / 4);

                int count = readFloatCount;
                while(count-- > 0)
                {
                   outFloatData[outFloatOffset++] = BitConverter.ToSingle(this.buffer, this.readPosition);
                    this.readPosition += 4;
                    if (this.readPosition == this.buffer.Length)
                    {
                        this.readPosition = 0;
                    }
                }
                this.numBytesInBuffer -= readFloatCount * 4;
                return readFloatCount;
            }
        }        
 


        /// <summary>
        /// Write bytes into the buffer.
        /// </summary>
        /// <param name="inByteData">Data to write</param>
        /// <param name="inByteOffset">Start offset into inByteData</param>
        /// <param name="writeByteCount">Number of bytes to write</param>
        /// <returns>number of bytes written</returns>
        public int WriteBytes(byte[] inByteData, int inByteOffset, int writeByteCount)
        {
            lock (lockObject)
            {
                writeByteCount = Math.Min(writeByteCount, inByteData.Length - inByteOffset);
                writeByteCount = Math.Min(writeByteCount, this.buffer.Length - this.numBytesInBuffer);

                int count = writeByteCount;
                while (count-- > 0)
                {
                    this.buffer[this.writePosition++] = inByteData[inByteOffset++];
                    if (this.writePosition == this.buffer.Length)
                    {
                        this.writePosition = 0;
                    }
                }
                this.numBytesInBuffer += writeByteCount;
                return writeByteCount;
            }
        }        

        /// <summary>
        /// Write floats into the buffer.
        /// </summary>
        /// <param name="inFloatData">Data to write</param>
        /// <param name="inFloatOffset">Start offset into inFloatData</param>
        /// <param name="floatWriteCount">Number of floats to write</param>
        /// <returns>number of floats written</returns>
        public int WriteFloats(float[] inFloatData, int inFloatOffset, int floatWriteCount)
        {
            lock (this.lockObject)
            {
                Debug.Assert((this.writePosition & 3) == 0);

                floatWriteCount = Math.Min(floatWriteCount, inFloatData.Length - inFloatOffset);
                floatWriteCount = Math.Min(floatWriteCount, (this.buffer.Length - this.numBytesInBuffer) / 4);

                int count = floatWriteCount;
                while( count-- > 0)
                {
                    ConvertFloatToBytes(inFloatData[inFloatOffset++], this.buffer, this.writePosition);
                    this.writePosition += 4;
                    if (this.writePosition >= this.buffer.Length)
                    {
                        this.writePosition = 0;
                    }
                }

                this.numBytesInBuffer += floatWriteCount * 4;
                return floatWriteCount;
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
        /// Maximum length of this circular buffer in bytes
        /// </summary>
        public int MaxBytes
        {
            get { return this.buffer.Length; }
        }

        /// <summary>
        /// Maximum length of this circular buffer in floats
        /// </summary>
        public int MaxFloats
        {
            get { return this.buffer.Length / 4; }
        }
        
        /// <summary>
        /// Number of bytes currently stored in the circular buffer
        /// </summary>
        public int ByteCount
        {
            get { return this.numBytesInBuffer; }
        }

        /// <summary>
        /// Number of floats currently stored in the circular buffer
        /// </summary>
        public int FloatCount
        {
            get { return this.numBytesInBuffer / 4; }
        }
        
        /// <summary>
        /// Resets the buffer
        /// </summary>
        public void Reset()
        {
            this.numBytesInBuffer = 0;
            this.readPosition = 0;
            this.writePosition = 0;
        }

        /// <summary>
        /// Advances the buffer, discarding bytes
        /// </summary>
        /// <param name="count">Bytes to advance</param>
        public void Advance(int bytesToAdvance)
        {
            if (bytesToAdvance >= this.numBytesInBuffer)
            {
                Reset();
            }
            else
            {
                this.numBytesInBuffer -= bytesToAdvance;
                this.readPosition += bytesToAdvance;
                this.readPosition %= MaxBytes;
            }
        }
    }
}

