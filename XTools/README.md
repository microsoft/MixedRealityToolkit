# HoloToolkit Sharing

## Description
The HoloToolkit.Sharing library allows applications to span multiple devices, and enables holographic collaboration.  

Originally developed for OnSight, a collaboration between SOTA (a Microsoft studio) and NASA to enhance their existing Mars rover planning tool with HoloLens, HoloToolkit.Sharing enables users to use multiple devices for a task by allowing the apps running on each device communicate and stay in sync seamlessly in real time.  

Users can also collaborate with other users (who are also using multiple devices) who may be in the same room or working remotely.  

## Features

### Multi-Platform, Multi-language
Enhance any app, on any device, written in any programming language
* Portable C++ code
* Auto-Generated bindings to most languages (SWIG).  C++, C#, and Java bindings are currently implemented, but it is possible to generate bindings for Python, Ruby, and more
* Currently Supported platforms: Win32, UWP, and OSX.  Support for Xbox One is available upon request

### Lobby & Session system
* Discover available sessions or create your own
* Permanent or ‘until empty’ session lifetime
* See user status: current session, mute state
* Easy to discover and hop between sessions

### Synchronization System
Synchronize data across all participants in session
* Everyone in session guaranteed to see the same thing
* Automatic conflict resolution for simultanious conflicting remote changes
* Real-time: See remote changes as they happen
* Shared data sets automatically merged when new users join a session
* Responsive – no delay in your own changes
* ‘Ownership’: your data leaves session when you do

### Group Voice Chat
Support for VOIP is built-in
* Server-based mixing lowers processing and bandwidth requirements for clients

### Visual Pairing
Connect devices just by looking at them
* One device displays a QR code with connection info and security code
* Other device sees QR code, connects, and validates with security code
* Can also detect location in 3D space using built-in fiducial marker support

### Profiler
Profiling and debugging an experience that spans multiple devices is challenging.  So HoloToolkit.Sharing provides an app that can connect to multiple devices at once and aggregates their timings and debug output in a single place

## Supported Languages and Platforms
* Win32, UWP, OSX
* C++, C#, Java
* x86, x64, ARM
