# Overview

- The SpectatorViewPlugin solution generates a dll needed for the SpectatorView experience in the MixedRealityToolkit-Unity repo:
  https://github.com/Microsoft/MixedRealityToolkit-Unity/tree/master/Assets/HoloToolkit-Preview/SpectatorView.
  
- SpectatorViewPlugin.dll utilizes OpenCV libraries for detecting ArUco markers.
  
- An example for how to call into this dll exists in the following unity script:
  https://github.com/Microsoft/MixedRealityToolkit-Unity/blob/master/Assets/HoloToolkit-Preview/SpectatorView/Scripts/SpatialSync/MarkerDetector.cs
  
- Logic within SpectatorViewPlugin.dll will only be executed on the HoloLens. It is not used on the mobile device, so you will only need to build x86 Release binaries. This can all be done on a PC in visual studio.

- After compiling this project, you will need to manually copy dlls from the compile output directory (${MixedRealityToolkit_RepoPath}\SpectatorViewPlugin\SpectatorViewPlugin\Release\SpectatorViewPlugin\) to a Plugin folder in your unity assets ($(MixedRealityToolkit-Unity_RepoPath}\Assets\Plugins\WSA\x86\).
>For more information on unity plugin directories, see https://docs.unity3d.com/Manual/SpecialFolders.html

# How to Build the SpectatorView Plugin for MRTK

## Install Vcpkg

- Open a Command Prompt in administrator mode
- Navigate to a folder in which you would like to store your repositories (ex: c:\git)
- git clone <https://github.com/Microsoft/vcpkg>
- cd vcpkg
- git checkout 05b31030cee412118a9710daf5b4652a684b7f50
>NOTE: The above commit was the last tested commit by a contributor to this repo. If the below setup steps fail with this commit, it is likely worth checking if the failure is a known issue for the vcpkg repo. It is also worth attempting checking out the master branch and reattempting the setup steps.
- .\bootstrap-vcpkg.bat
- .\vcpkg integrate install

## Install OpenCV Contrib for UWP

- .\vcpkg install opencv[contrib]:x86-uwp --recurse

>NOTE: Copy the above line exactly (the []s do not indicate an optional value).

## Building the Plugin

- Navigate to your Git repository folder (ex: c:\git)
- git clone <https://github.com/Microsoft/MixedRealityToolkit.git>
- Open the .sln file located in MixedRealityToolkit\SpectatorViewPlugin\SpectatorViewPlugin\SpectatorViewPlugin.sln
- Set the build options to Release and x86, then build the solution.

## Add the DLLs to your Project

Copy the following dlls from the directory MixedRealityToolkit\SpectatorViewPlugin\SpectatorViewPlugin\Release\SpectatorViewPlugin to the directory HoloToolkit-Preview\SpectatorView\Plugins\WSA\x86. If that directory doesn't exist please create it.

- opencv_aruco343.dll
- opencv_calib3d343.dll
- opencv_core343.dll
- opencv_features2d343.dll
- opencv_flann343.dll
- opencv_imgproc343.dll
- zlib1.dll
- SpectatorViewPlugin.dll

# Troubleshooting
## Installing OpenCV Contrib for UWP failed

The suggested commit above for vcpkg reflects the last locally tested vcpkg commit by a contributor to this repo. When encountering issues with vcpkg, it is likely worth checking out the master branch for the vcpkg repo and repeating the vcpkg related steps. Issues may also already be filed for issues with vcpkg.
>NOTE: When trying other vcpkgs commits, you may end up with a different version of opencv getting installed to your development machine. This will likely require updating the opencv lib dependencies as described below.

## OpenCV header/dll is not found

If installing opencv with vcpkg succeeded, a few things could still occur that prevent SpectatorViewPlugin's from referencing the opencv libs/dlls correctly. Try the following:

- Restart Visual Studio. If SpectatorViewPlugin.sln was opened in visual studio prior to installing the opencv for uwp components, visual studio may not have correctly resolved needed environment variables. Closing and reopening visual studio should result in these environment variable paths resolving correctly.
- Ensure that the opencv lib dependencies declared in the SpectatorViewPlugin project have the correct version number. Vcpkg will periodically move to installing newer versions of opencv. If you right click on your SpectatorViewPlugin project in visual studio's solution explorer, you can then open the project properties dialogue. Look at Linker->Input to see what specific opencv libs are referenced by the project. For OpenCV 3.4.3, you will need to make sure that libs end in *343.lib*. Older versions of OpenCV, such as 3.4.1, have dlls ending in *341.lib*
