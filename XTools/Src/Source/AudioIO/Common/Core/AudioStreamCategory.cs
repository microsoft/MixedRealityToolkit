using System;

namespace AudioIO_DLL.IO
{
    /// <summary>
    /// Redefining the C++ AUDIO_STREAM_CATEGORY enum declared 
    /// in AudioSessionTypes.h from the Windows 8.1 platform APIs
    /// </summary>
    public enum AudioStreamCategory : int
    {
        // AudioCategory_Other
        Other = 0,

        //AudioCategory_ForegroundOnlyMedia
        ForegroundOnlyMedia,

        // AudioCategory_BackgroundCapableMedia
        BackgroundCapableMedia,

        // AudioCategory_Communications
        Communications,

        // AudioCategory_Alerts
        Alerts,

        // AudioCategory_SoundEffects
        SoundEffects,

        // AudioCategory_GameEffects
        GameEffects,

        // AudioCategory_GameMedia
        GameMedia
    }
}
