using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using AudioIO_DLL.IO;

namespace AudioIO_DLL.Core.Interfaces
{

    [ComImport, InterfaceType(ComInterfaceType.InterfaceIsIUnknown), Guid("726778CD-F60A-4eda-82DE-E47610CD78AA")]
    interface IAudioClient2
    {
        [PreserveSig]
        int Initialize(AudioClientShareMode shareMode,
                       AudioClientStreamFlags streamFlags,
                       long hnsBufferDuration, // REFERENCE_TIME
                       long hnsPeriodicity, // REFERENCE_TIME
                       [In] WaveFormatExtensible pFormat,
                       [In] IntPtr audioSessionGuid);

        // ref Guid AudioSessionGuid

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
            [In] WaveFormatExtensible pFormat,
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
        int GetService([In, MarshalAs(UnmanagedType.LPStruct)] Guid interfaceId,
                       [Out, MarshalAs(UnmanagedType.IUnknown)] out object interfacePointer);

        //virtual HRESULT STDMETHODCALLTYPE IsOffloadCapable(/*[in]*/ _In_  
        //   AUDIO_STREAM_CATEGORY Category, /*[in]*/ _Out_  BOOL *pbOffloadCapable) = 0;
        void IsOffloadCapable(int category, out bool pbOffloadCapable);

        //virtual HRESULT STDMETHODCALLTYPE SetClientProperties(/*[in]*/ _In_  
        //  const AudioClientProperties *pProperties) = 0;
        [PreserveSig]
        int SetClientProperties([In] ref AudioClientProperties pProperties);

        //virtual HRESULT STDMETHODCALLTYPE GetBufferSizeLimits(/*[in]*/ _In_  
        //   const WAVEFORMATEX *pFormat, /*[in]*/ _In_  BOOL bEventDriven, /*[in]*/ 
        //  _Out_  REFERENCE_TIME *phnsMinBufferDuration, /*[in]*/ _Out_  
        //  REFERENCE_TIME *phnsMaxBufferDuration) = 0;
        void GetBufferSizeLimits(IntPtr pFormat, bool bEventDriven,
                                 out long phnsMinBufferDuration, out long phnsMaxBufferDuration);
    }
}
