# DLL for .NET Native bug workaround

This solution provides a workaround for a .NET Native (used when building a Master build for a Unity app built with the .NET backend) bug, where `IInspectable` pointers cannot be marshaled from native to managed code using `Marshal.GetObjectForIUnknown`.

To build for all Release flavors, run `build.bat` in the Visual Studio Developer Command Prompt for VS.
Otherwise, open the solution in Visual Studio and build with your desired settings.

Drop this into your Unity app in a Plugins folder, and the [Windows Mixed Reality Utilities script](https://github.com/microsoft/MixedRealityToolkit-Unity/blob/mrtk_development/Assets/MixedRealityToolkit.Providers/WindowsMixedReality/WindowsMixedRealityUtilities.cs#L20) will resolve properly.
