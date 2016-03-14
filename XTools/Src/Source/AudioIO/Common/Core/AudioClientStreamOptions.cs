using System;

namespace AudioIO_DLL.IO
{
    /// <summary>
    /// Redefining the C++ AUDCLNT_STREAMOPTIONS enum declared 
    /// in AudioSessionTypes.h from the Windows 8.1 platform APIs
    /// </summary>
    public enum AudioClientStreamOptions : int
    {
        //AUDCLNT_STREAMOPTIONS_NONE = 0,
        None = 0,

        //AUDCLNT_STREAMOPTIONS_RAW = 0x1
        Raw = 0x1
    }
}