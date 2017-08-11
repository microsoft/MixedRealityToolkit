![](https://github.com/Microsoft/MixedRealityToolkit-Unity/blob/master/External/ReadMeImages/MRTK_Logo_Rev.png)

# MixedRealityToolkit (MRTK)
The mixed reality toolkit is a collection of scripts and components intended to accelerate the development of applications targeting Windows Mixed Reality.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). 
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

# MixedRealityToolkit for Unity
If you are developing your app using Unity - please visit [MixedRealityToolkit-Unity](https://github.com/Microsoft/MixedRealityToolkit-Unity).

---

## Spatial Mapping

### Plane Finding
Standalone Visual Studio solution containing the source code for the PlaneFinding DLL. Building this solution produces two variants of the DLL (one for use in the Unity Editor, and another for use at runtime on a HoloLens), along with a few simple tests for sanity testing changes to the code.

---

## Spatial Understanding

Standalone Visual Studio solution containing the source code for the SpatialUnderstanding DLL. 

SpatialUnderstanding library encapsulates the world understanding technology by for Conker and Fragments. It allows you to quickly find empty spaces on the walls, place objects on the ceiling, identify placed for character to sit, and a myriad of other spatial understanding queries. 

There are three primary interfaces exposed by the module: topology for simple surface and spatial queries, shape for object detection, and the object placement solver for constraint based placement of object sets. Each of these is described below.

In addition to the three primary module interfaces, a ray casting interface can be used to retrieve tagged surface types and a custom watertight playspace mesh can be copied out.

---

## Sharing

The HoloToolkit.Sharing library allows applications to span multiple devices, and enables collaboration between users in the same room or working remotely.  Features include:

* Runs on any platform, and can work with any programming language
* Lobby & Session system
* Synchronization System
* Visual Pairing
* Anchor Sharing
* Profiler

[Learn More...](Sharing/README.md)

---

## Microphone Stream Selector

The MicStreamSelector library allows applications to easily access the different Microphone Stream Categories of any windows 10 device. On HoloLens, those capture types can be optimized for either: high-quality speech capture, VOIP transmission, or general room captures. The library can record wav files of indeterminate length, and it also allows realtime access to the mic data. The microphone audio data can be polled as desired (which works well for game engines that are running on a framerate, like Unity), or can be provided via an embedded callback in your app to deliver realtime data as it is available (in a state driven app, like flat XAML).

The provided XAML demo app is a uses the embedded callback method. It shows how to use the raw data from the microphone and also how to record wav files. https://github.com/Microsoft/HoloToolkit-Unity/blob/master/Assets/HoloToolkit/Input/Tests/Scripts/MicStreamDemo.cs has an alternate example showing how to use this library inside of Unity. 

To use, you want to call the proper MicInitialize function for your app, whether you will poll the data yourself every frame, like in Unity, or whether you want the plugin to provide the data to the app whenever it is ready, like in the provided XAML example. After initialization, you can call any of the functions in any order. When you are closing your application, you want to call MicDestroy() to properly deconstruct. If you have called MicDestroy() already in your app, you need to call MicInitialize before using again. 

This plugin assumes categories where { SPEECH=0, COMMUNICATIONS=1, MEDIA=2 }. On HoloLens: SPEECH is low-quality, beam-formed voice. COMMUNICATIONS is high-quality, beam-formed voice. MEDIA is a general room capture. 

### Initialize & poll audio for Game Engines (e.g. Unity)
* MicInitializeDefault(int category);
Starts microphone for selected category, as defined above. Uses default settings for Audio Device. 

* MicInitializeCustomRate(int category, int samplerate);
Same as above, but uses custom samplerate. 

* MicGetFrame(float[] buffer, int length, int numchannels); 
Fills buffer of length and numchannels. This is how Unity3d and Unreal will want to access audio data -- from a poll in the game engine's audio engine. This is used in the example from https://github.com/Microsoft/HoloToolkit-Unity/blob/master/Assets/HoloToolkit/Input/Tests/Scripts/MicStreamDemo.cs

### Initialize with callback for passive, state-driven apps (e.g. XAML)
* MicInitializeDefaultWithGraph(int category, AudioGraph appGraph); 
Same as default, but in this case you must declare the AudioGraph in the app before passing to the plugin. 

* MicInitializeCustomRateWithGraph(int category, int samplerate, AudioGraph appGraph);
Same as above, but uses custom samplerate.

When using this method, you should almost definitely be providing a micsignal callback delegate function when calling MicStartStream, as defined below and is as shown in the provided example. This delegate function is called every time a frame of audio is ready from the device. 

### Universal APIs
* MicStartStream(bool keepData, bool previewOnDevice, LiveMicCallback micsignal); 
Starts streaming raw audio data from microphone. keepData will never drop data while recording and could cause huge memory use if used incorrectly -- defaults to false. previewOnDevice, if true, will playback the audio in realtime for monitoring/listening -- doesn't affect the actual acquisition of data. micsignal is a delegate function called by the plugin whenever audio data is ready -- should generally be used by state-driven apps (XAML) and not game engines (Unity).

* MicStopStream();
Stops the raw audio streaming. Won't stop a recording, if in progress. 

* MicStartRecording(string filename, bool previewOnDevice); 
filename expects extension, e.g. "MySound.wav". previewOnDevice will play the audio stream in speakers exactly as MicStartStream would -- call MicStopStream to mute after using. 

* MicStopRecording(StringBuilder sb);
sb returns the full path on disk to the wav file you just recorded. 

* MicDestroy();
Releases everything -- call when closing application or not using Microphone for long period of time. 

* MicPause();
Literally pauses everthing happening -- useful for minimizing or backgrounding. 

* MicResume();
Resumes from paused state.

* MicSetGain(float gain);
Can call anytime. Default is 1, 0 is muted, and can go to +inifinity although that would be painful. 

* MicGetDefaultBufferSize(); 
If you did a default setup, you might want to know how much data to expect from the audio device in one chunk. 

* MicGetDefaultNumChannels();
If you did a default setup, you might want to know how many channels are in your microphone stream.

---

# Useful resources on Microsoft Windows Dev Center
| ![Academy](https://github.com/Microsoft/MixedRealityToolkit-Unity/blob/master/External/ReadMeImages/icon_academy.png) [Academy](https://developer.microsoft.com/en-us/windows/mixed-reality/academy)| ![Design](https://github.com/Microsoft/MixedRealityToolkit-Unity/blob/master/External/ReadMeImages/icon_design.png) [Design](https://developer.microsoft.com/en-us/windows/mixed-reality/design)| ![Development](https://github.com/Microsoft/MixedRealityToolkit-Unity/blob/master/External/ReadMeImages/icon_development.png) [Development](https://developer.microsoft.com/en-us/windows/mixed-reality/development)| ![Community)](https://github.com/Microsoft/MixedRealityToolkit-Unity/blob/master/External/ReadMeImages/icon_community.png) [Community](https://developer.microsoft.com/en-us/windows/mixed-reality/community)|
| :--------------------- | :----------------- | :------------------ | :------------------------ |
| See code examples. Do a coding tutorial. Watch guest lectures.          | Get design guides. Build user interface. Learn interactions and input.     | Get development guides. Learn the technology. Understand the science.       | Join open source projects. Ask questions on forums. Attend events and meetups. |

