
public class BufferConstants
{
    /// <summary>
    /// the HRTF buffer size const
    /// </summary>
    public const int HRTFBufferDLLSize = 480; // normally 1024;

    /// <summary>
    /// the base frequency for both recording and playback
    /// </summary>
    public const int BaseFrequency = 48000;

    /// <summary>
    /// AudioSink.halfSecond data points is 1/2 of a second in samples
    /// </summary>
    public const int HalfSecond = BaseFrequency / 2;

    /// <summary>
    /// AudioSink.halfSecond data points is 1/4 of a second in samples and 
    /// must be a multiple of 4!
    /// </summary>
    public const int QuarterSecond = (BaseFrequency / 16) * 4;

    /// <summary>
    /// number of audio channels before HRTF
    /// </summary>
    public const int AudioChannelsPreHRTF = 1;    
    
    /// <summary>
    /// number of audio channels after HRTF
    /// </summary>
    public const int AudioChannelsAfterHRTF = 2;
    
    /// <summary>
    /// don't start to deliver any audio until this time has passed so we are sure to 
    /// have some data in the buffer.  When we do begin to send data, we ignore old audio data 
    /// </summary>
    public const int DeliveryLatency = BaseFrequency / 30; 
   
    /// <summary>
    /// no audio buffer size (1/20th of a second)
    /// </summary>
    public const int NullAudioBufferSize = BaseFrequency / 20;
    
    /// <summary>
    /// target audio buffer latency in milliseconds - not guaranteed
    /// </summary>
    public const int AudioBufferLatency = 10;

    /// <summary>
    /// Its unclear exactly what this value represents, but it was a magic number used in several locations, so refactored it to here.  
    /// The nearby comment "this avoids network fragmentation and sends the biggest packet possible without intervention"
    /// makes me suspect that this is supposed to represent maximum audio buffer size that can be sent without being split into
    /// two network packets (ethernet has an MTU of 1492).  
    /// </summary>
    public const int UnknownBufferValue01 = 1452;
}