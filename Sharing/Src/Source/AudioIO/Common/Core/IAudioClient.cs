using System;
using System.Runtime.InteropServices;
using AudioIO_DLL.IO;

namespace AudioIO_DLL.Core.Interfaces
{
    /// <summary>
    /// Defined in AudioClient.h
    /// </summary>
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown), Guid("1CB9AD4C-DBFA-4c32-B178-C2F568A703B2")]
    internal interface IAudioClient
    {
        [PreserveSig]
        int Initialize(AudioClientShareMode shareMode,
            AudioClientStreamFlags StreamFlags,
            long hnsBufferDuration, // REFERENCE_TIME
            long hnsPeriodicity, // REFERENCE_TIME
            [In] ref WaveFormatExtensible pFormat,
            [In] ref Guid AudioSessionGuid);

        /// <summary>
        /// The GetBufferSize method retrieves the size (maximum capacity) of the endpoint buffer.
        /// </summary>
        int GetBufferSize(out uint bufferSize);

        [return: MarshalAs(UnmanagedType.I8)]
        long GetStreamLatency();

        int GetCurrentPadding(out int currentPadding);

        [PreserveSig]
        int IsFormatSupported(
            AudioClientShareMode shareMode,
            [In] ref WaveFormatExtensible pFormat,
            [Out, MarshalAs(UnmanagedType.LPStruct)] out WaveFormatExtensible closestMatchFormat);
        
        int GetMixFormat(out IntPtr deviceFormatPointer);

        // REFERENCE_TIME is 64 bit int        
        int GetDevicePeriod(out long defaultDevicePeriod, out long minimumDevicePeriod);

        int Start();

        int Stop();

        int Reset();
        
        int SetEventHandle(IntPtr eventHandle);

        /// <summary>
        /// The GetService method accesses additional services from the audio client object.
        /// </summary>
        /// <param name="interfaceId">The interface ID for the requested service.</param>
        /// <param name="interfacePointer">Pointer to a pointer variable into which the method writes the address of an instance of the requested interface. </param>
        [PreserveSig]
        int GetService([In, MarshalAs(UnmanagedType.LPStruct)] Guid interfaceId, [Out, MarshalAs(UnmanagedType.IUnknown)] out object interfacePointer);
    }
}
