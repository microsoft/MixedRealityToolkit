@echo off
@setlocal

if not exist "%HoloToolkitUnityRoot%" set HoloToolkitUnityRoot=%1
if "x%HoloToolkitUnityRoot%" == "x" goto ErrorNoHoloToolkitUnityTarget
if not exist "%HoloToolkitUnityRoot%\Assets" goto ErrorHoloToolkitUnityTargetInvalid
if not exist "%HoloToolkitUnityRoot%\External" goto ErrorHoloToolkitUnityTargetInvalid

xcopy SDK\Client\CSharp\API\* "%UnityRoot%\Assets\HoloToolkit\Sharing\Scripts\SDK\" /S /I /F /H /R /K /Y

xcopy SDK\Client\CSharp\bin\Windows\x86\SharingClient.dll "%UnityRoot%\Assets\HoloToolkit\Sharing\Plugins\x86\" /S /I /F /H /R /K /Y
xcopy SDK\Client\CSharp\bin\Windows\x64\SharingClient.dll "%UnityRoot%\Assets\HoloToolkit\Sharing\Plugins\x64\" /S /I /F /H /R /K /Y

xcopy SDK\Client\CSharp\bin\UWP\ARM\SharingClient.dll "%UnityRoot%\Assets\HoloToolkit\Sharing\Plugins\WSA\ARM\" /S /I /F /H /R /K /Y
xcopy SDK\Client\CSharp\bin\UWP\x86\SharingClient.dll "%UnityRoot%\Assets\HoloToolkit\Sharing\Plugins\WSA\x86\" /S /I /F /H /R /K /Y
xcopy SDK\Client\CSharp\bin\UWP\x64\SharingClient.dll "%UnityRoot%\Assets\HoloToolkit\Sharing\Plugins\WSA\x64\" /S /I /F /H /R /K /Y

xcopy SDK\Server\SharingService.exe "%UnityRoot%\External\HoloToolkit\Sharing\Server\" /S /I /F /H /R /K /Y

xcopy SDK\Tools\Profiler\x64\ProfilerX.exe "%UnityRoot%\External\HoloToolkit\Sharing\Tools\Profiler\x64\" /S /I /F /H /R /K /Y
xcopy SDK\Tools\Profiler\x64\SharingProfiler.dll "%UnityRoot%\External\HoloToolkit\Sharing\Tools\Profiler\x64\" /S /I /F /H /R /K /Y

xcopy SDK\Tools\Profiler\x86\ProfilerX.exe "%UnityRoot%\External\HoloToolkit\Sharing\Tools\Profiler\x86\" /S /I /F /H /R /K /Y
xcopy SDK\Tools\Profiler\x86\SharingProfiler.dll "%UnityRoot%\External\HoloToolkit\Sharing\Tools\Profiler\x86\" /S /I /F /H /R /K /Y

xcopy SDK\Tools\SessionManager\x64\SessionManager.UI.exe "%UnityRoot%\External\HoloToolkit\Sharing\Tools\SessionManager\x64\" /S /I /F /H /R /K /Y
xcopy SDK\Tools\SessionManager\x64\SharingClient.dll "%UnityRoot%\External\HoloToolkit\Sharing\Tools\SessionManager\x64\" /S /I /F /H /R /K /Y

xcopy SDK\Tools\SessionManager\x86\SessionManager.UI.exe "%UnityRoot%\External\HoloToolkit\Sharing\Tools\SessionManager\x86\" /S /I /F /H /R /K /Y
xcopy SDK\Tools\SessionManager\x86\SharingClient.dll "%UnityRoot%\External\HoloToolkit\Sharing\Tools\SessionManager\x86\" /S /I /F /H /R /K /Y

exit /b 0

:ErrorNoHoloToolkitUnityTarget
echo Target path us required. Set the HoloToolkitUnityRoot environment variable or pass the root of the target Unity project as the first argument to this script.
exit /b 1

:ErrorHoloToolkitUnityTargetInvalid
echo Target path is not a valid Unity project, verify that the Assets and External directories exists.
exit /b 1