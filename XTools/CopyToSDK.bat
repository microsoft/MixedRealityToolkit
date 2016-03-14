@echo off

xcopy Src\bin\Release\Win32\ClientWindowsCSharp\SharingClient.dll SDK\Client\CSharp\Windows\x86\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\x64\ClientWindowsCSharp\SharingClient.dll SDK\Client\CSharp\Windows\x64\ /S /I /F /H /R /K /Y

xcopy Src\bin\Release\ARM\ClientUniversalCSharp\SharingClient.dll SDK\Client\CSharp\UWP\ARM\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\Win32\ClientUniversalCSharp\SharingClient.dll SDK\Client\CSharp\UWP\x86\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\x64\ClientUniversalCSharp\SharingClient.dll SDK\Client\CSharp\UWP\x64\ /S /I /F /H /R /K /Y

xcopy Src\GeneratedInterface\CSharpAPI\* SDK\Client\CSharp\ /S /I /F /H /R /K /Y
xcopy Src\Source\Helpers\CSharp\* SDK\Client\CSharp\ /S /I /F /H /R /K /Y

xcopy Src\GeneratedInterface\SideCarAPI\* SDK\Client\SideCar\ /S /I /F /H /R /K /Y

xcopy Src\bin\Release\Win32\ClientWindowsJava\SharingClient.dll SDK\Client\Java\Windows\x86\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\x64\ClientWindowsJava\SharingClient.dll SDK\Client\Java\Windows\x64\ /S /I /F /H /R /K /Y

xcopy Src\GeneratedInterface\JavaAPI\* SDK\Client\Java\ /S /I /F /H /R /K /Y

xcopy Src\bin\Release\x64\SharingService\SharingService.exe SDK\Server\ /S /I /F /H /R /K /Y

xcopy Src\bin\Release\Win32\ProfilerX\* SDK\Tools\Profiler\x86\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\x64\ProfilerX\* SDK\Tools\Profiler\x64\ /S /I /F /H /R /K /Y

xcopy Src\bin\Release\Win32\SessionManager.UI\* SDK\Tools\SessionManager\x86\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\x64\SessionManager.UI\* SDK\Tools\SessionManager\x64\ /S /I /F /H /R /K /Y
