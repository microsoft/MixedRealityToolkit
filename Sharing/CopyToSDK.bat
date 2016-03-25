@echo off

xcopy Src\bin\Release\Win32\ClientWindowsCSharp\SharingClient.dll SDK\Client\CSharp\bin\Windows\x86\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\x64\ClientWindowsCSharp\SharingClient.dll SDK\Client\CSharp\bin\Windows\x64\ /S /I /F /H /R /K /Y

xcopy Src\bin\Release\ARM\ClientUniversalCSharp\SharingClient.dll SDK\Client\CSharp\bin\UWP\ARM\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\Win32\ClientUniversalCSharp\SharingClient.dll SDK\Client\CSharp\bin\UWP\x86\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\x64\ClientUniversalCSharp\SharingClient.dll SDK\Client\CSharp\bin\UWP\x64\ /S /I /F /H /R /K /Y

xcopy Src\GeneratedInterface\CSharpAPI\* SDK\Client\CSharp\API\ /S /I /F /H /R /K /Y
xcopy Src\Source\Helpers\CSharp\* SDK\Client\CSharp\API\ /S /I /F /H /R /K /Y

xcopy Src\GeneratedInterface\SideCarAPI\* SDK\Client\SideCar\include\ /S /I /F /H /R /K /Y

xcopy Src\bin\Release\Win32\ClientWindowsJava\SharingClient.dll SDK\Client\Java\bin\Windows\x86\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\x64\ClientWindowsJava\SharingClient.dll SDK\Client\Java\bin\Windows\x64\ /S /I /F /H /R /K /Y

xcopy Src\GeneratedInterface\JavaAPI\* SDK\Client\Java\API\ /S /I /F /H /R /K /Y

xcopy Src\bin\Release\x64\SharingService\SharingService.exe SDK\Server\ /S /I /F /H /R /K /Y

xcopy Src\bin\Release\Win32\ProfilerX\ProfilerX.exe SDK\Tools\Profiler\x86\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\Win32\ProfilerX\SharingProfiler.dll SDK\Tools\Profiler\x86\ /S /I /F /H /R /K /Y

xcopy Src\bin\Release\x64\ProfilerX\ProfilerX.exe SDK\Tools\Profiler\x64\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\x64\ProfilerX\SharingProfiler.dll SDK\Tools\Profiler\x64\ /S /I /F /H /R /K /Y

xcopy Src\bin\Release\Win32\SessionManager.UI\SessionManager.UI.exe SDK\Tools\SessionManager\x86\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\Win32\SessionManager.UI\SharingClient.dll SDK\Tools\SessionManager\x86\ /S /I /F /H /R /K /Y

xcopy Src\bin\Release\x64\SessionManager.UI\SessionManager.UI.exe SDK\Tools\SessionManager\x64\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\x64\SessionManager.UI\SharingClient.dll SDK\Tools\SessionManager\x64\ /S /I /F /H /R /K /Y
