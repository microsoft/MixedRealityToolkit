@echo off
@setlocal

if not exist "%MixedRealityToolkitUnityRoot%" set MixedRealityToolkitUnityRoot=%1
if "x%MixedRealityToolkitUnityRoot%" == "x" goto ErrorNoMixedRealityToolkitUnityTarget
if not exist "%MixedRealityToolkitUnityRoot%\Assets" goto ErrorMixedRealityToolkitUnityTargetInvalid
if not exist "%MixedRealityToolkitUnityRoot%\External" goto ErrorMixedRealityToolkitUnityTargetInvalid

xcopy SDK\Client\CSharp\API\* "%MixedRealityToolkitUnityRoot%\Assets\MixedRealityToolkit\Sharing\Scripts\SDK\" /S /I /F /H /R /K /Y

xcopy SDK\Client\CSharp\bin\Windows\x86\SharingClient.dll "%MixedRealityToolkitUnityRoot%\Assets\MixedRealityToolkit\Sharing\Plugins\x86\" /S /I /F /H /R /K /Y
xcopy SDK\Client\CSharp\bin\Windows\x64\SharingClient.dll "%MixedRealityToolkitUnityRoot%\Assets\MixedRealityToolkit\Sharing\Plugins\x64\" /S /I /F /H /R /K /Y

xcopy SDK\Client\CSharp\bin\UWP\ARM\SharingClient.dll "%MixedRealityToolkitUnityRoot%\Assets\MixedRealityToolkit\Sharing\Plugins\WSA\ARM\" /S /I /F /H /R /K /Y
xcopy SDK\Client\CSharp\bin\UWP\x86\SharingClient.dll "%MixedRealityToolkitUnityRoot%\Assets\MixedRealityToolkit\Sharing\Plugins\WSA\x86\" /S /I /F /H /R /K /Y
xcopy SDK\Client\CSharp\bin\UWP\x64\SharingClient.dll "%MixedRealityToolkitUnityRoot%\Assets\MixedRealityToolkit\Sharing\Plugins\WSA\x64\" /S /I /F /H /R /K /Y

xcopy SDK\Server\SharingService.exe "%MixedRealityToolkitUnityRoot%\External\MixedRealityToolkit\Sharing\Server\" /S /I /F /H /R /K /Y

xcopy SDK\Tools\Profiler\x64\ProfilerX.exe "%MixedRealityToolkitUnityRoot%\External\MixedRealityToolkit\Sharing\Tools\Profiler\x64\" /S /I /F /H /R /K /Y
xcopy SDK\Tools\Profiler\x64\SharingProfiler.dll "%MixedRealityToolkitUnityRoot%\External\MixedRealityToolkit\Sharing\Tools\Profiler\x64\" /S /I /F /H /R /K /Y

xcopy SDK\Tools\Profiler\x86\ProfilerX.exe "%MixedRealityToolkitUnityRoot%\External\MixedRealityToolkit\Sharing\Tools\Profiler\x86\" /S /I /F /H /R /K /Y
xcopy SDK\Tools\Profiler\x86\SharingProfiler.dll "%MixedRealityToolkitUnityRoot%\External\MixedRealityToolkit\Sharing\Tools\Profiler\x86\" /S /I /F /H /R /K /Y

xcopy SDK\Tools\SessionManager\x64\SessionManager.UI.exe "%MixedRealityToolkitUnityRoot%\External\MixedRealityToolkit\Sharing\Tools\SessionManager\x64\" /S /I /F /H /R /K /Y
xcopy SDK\Tools\SessionManager\x64\SharingClient.dll "%MixedRealityToolkitUnityRoot%\External\MixedRealityToolkit\Sharing\Tools\SessionManager\x64\" /S /I /F /H /R /K /Y

xcopy SDK\Tools\SessionManager\x86\SessionManager.UI.exe "%MixedRealityToolkitUnityRoot%\External\MixedRealityToolkit\Sharing\Tools\SessionManager\x86\" /S /I /F /H /R /K /Y
xcopy SDK\Tools\SessionManager\x86\SharingClient.dll "%MixedRealityToolkitUnityRoot%\External\MixedRealityToolkit\Sharing\Tools\SessionManager\x86\" /S /I /F /H /R /K /Y

exit /b 0

:ErrorNoMixedRealityToolkitUnityTarget
echo Target path is required. Set the MixedRealityToolkitUnityRoot environment variable or pass the root of the target Unity project as the first argument to this script.
exit /b 1

:ErrorMixedRealityToolkitUnityTargetInvalid
echo Target path is not a valid Unity project, verify that the Assets and External directories exists.
exit /b 1