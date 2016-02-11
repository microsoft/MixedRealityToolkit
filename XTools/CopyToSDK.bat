@echo off

xcopy Src\bin\Release\Win32\ClientWindowsCSharp\XToolsClient.dll SDK\Client\CSharp\Windows\x86\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\x64\ClientWindowsCSharp\XToolsClient.dll SDK\Client\CSharp\Windows\x64\ /S /I /F /H /R /K /Y

xcopy Src\bin\Release\ARM\ClientUniversalCSharp\XToolsClient.dll SDK\Client\CSharp\UWP\ARM\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\Win32\ClientUniversalCSharp\XToolsClient.dll SDK\Client\CSharp\UWP\x86\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\x64\ClientUniversalCSharp\XToolsClient.dll SDK\Client\CSharp\UWP\x64\ /S /I /F /H /R /K /Y

xcopy Src\GeneratedInterface\CSharpAPI\* SDK\Client\CSharp\ /S /I /F /H /R /K /Y
xcopy Src\Source\Helpers\CSharp\* SDK\Client\CSharp\ /S /I /F /H /R /K /Y

xcopy Src\GeneratedInterface\SideCarAPI\* SDK\Client\SideCar\ /S /I /F /H /R /K /Y

xcopy Src\bin\Release\Win32\ClientWindowsJava\XToolsClient.dll SDK\Client\Java\Windows\x86\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\x64\ClientWindowsJava\XToolsClient.dll SDK\Client\Java\Windows\x64\ /S /I /F /H /R /K /Y

xcopy Src\GeneratedInterface\JavaAPI\* SDK\Client\Java\ /S /I /F /H /R /K /Y

xcopy Src\bin\Release\x64\SessionServer\SessionServer.exe SDK\Server\ /S /I /F /H /R /K /Y

xcopy Src\bin\Release\Win32\ProfilerX\* SDK\Tools\Profiler\x86\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\x64\ProfilerX\* SDK\Tools\Profiler\x64\ /S /I /F /H /R /K /Y

xcopy Src\bin\Release\Win32\SessionManager.UI\* SDK\Tools\SessionManager\x86\ /S /I /F /H /R /K /Y
xcopy Src\bin\Release\x64\SessionManager.UI\* SDK\Tools\SessionManager\x64\ /S /I /F /H /R /K /Y
