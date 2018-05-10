How to Build the SpectatorView Plugin for MRTK
==============================================

Install Vcpkg
---------------
    - Open a Command Prompt
    - git clone https://github.com/Microsoft/vcpkg
    - cd vcpkg
    - .\bootstrap-vcpkg.bat
    - .\vcpkg integrate install (note: requires admin)

Install OpenCV Contrib for UWP
------------------------------
    - .\vcpkg install opencv[contrib]:x86-uwp --recurse

Building the Plugin
-------------------
    - git clone https://github.com/Microsoft/MixedRealityToolkit.git
    - Open the .sln file located in MixedRealityToolkit\ARCA\OpenCVWrapper\OpenCVWrapper.sln
    - Set the build options to Release and build the solution.
    - Copy the following dlls from the directory MixedRealityToolkit\SpectatorView\OpenCVWrapper\Release\OpenCVWrapper to the directory HoloToolkit\SpectatorView\Plugins\WSA\x86\
    - opencv_aruco341.dll
    - opencv_calib3d341.dll
    - opencv_core341.dll
    - opencv_features2d341.dll
    - opencv_flann341.dll
    - opencv_imgproc341.dll
    - zlib1.dll
    - OpenCVWrapper.dll