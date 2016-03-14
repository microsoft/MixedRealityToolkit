@echo off

if exist .\builds\ rd /S /Q .\builds\

start "" /wait cmd.exe /C Build_x64.bat
start "" /wait cmd.exe /C Build_x86.bat

xcopy builds\libcurl-vc12-x64-debug-static-ipv6-sspi-winssl\lib\*.lib ..\lib\Debug\x64\ /I /F /H /R /K /Y
xcopy builds\libcurl-vc12-x64-debug-static-ipv6-sspi-winssl\lib\*.pdb ..\lib\Debug\x64\ /I /F /H /R /K /Y

xcopy builds\libcurl-vc12-x64-release-static-ipv6-sspi-winssl\lib\*.lib ..\lib\Release\x64\ /I /F /H /R /K /Y
xcopy builds\libcurl-vc12-x64-release-static-ipv6-sspi-winssl\lib\*.pdb ..\lib\Release\x64\ /I /F /H /R /K /Y

xcopy builds\libcurl-vc12-x86-debug-static-ipv6-sspi-winssl\lib\*.lib ..\lib\Debug\Win32\ /I /F /H /R /K /Y
xcopy builds\libcurl-vc12-x86-debug-static-ipv6-sspi-winssl\lib\*.pdb ..\lib\Debug\Win32\ /I /F /H /R /K /Y

xcopy builds\libcurl-vc12-x86-release-static-ipv6-sspi-winssl\lib\*.lib ..\lib\Release\Win32\ /I /F /H /R /K /Y
xcopy builds\libcurl-vc12-x86-release-static-ipv6-sspi-winssl\lib\*.pdb ..\lib\Release\Win32\ /I /F /H /R /K /Y

xcopy builds\libcurl-vc12-x64-debug-static-ipv6-sspi-winssl\include\* ..\include\ /S /I /F /H /R /K /Y

exit /b 0
