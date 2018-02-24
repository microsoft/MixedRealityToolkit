@echo off

REM Save current directory
set curdir=%cd%

REM IMPORTANT: You will likely need to set this environment variable manually. Visual Studio no longer provides environment variables by default. Path should be similar to C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools
@if "%DevEnvDir%"=="" call "%VS150COMNTOOLS%VsDevCmd.bat"

REM Restore current directory
cd /d %curdir%

call MSBuild win32/zint.sln /p:Configuration=Debug;Platform=ARM /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild win32/zint.sln /p:Configuration=Release;Platform=ARM /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild win32/zint.sln /p:Configuration=Debug;Platform=Win32 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild win32/zint.sln /p:Configuration=Release;Platform=Win32 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild win32/zint.sln /p:Configuration=Debug;Platform=x64 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild win32/zint.sln /p:Configuration=Release;Platform=x64 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

xcopy build\* ..\lib\ /I /F /H /R /K /Y /S

xcopy backend\*.h ..\include\ /I /F /H /R /K /Y /S
xcopy backend\*.hh ..\include\ /I /F /H /R /K /Y /S

:End
echo ***********************************
echo ********* Build Succeeded *********
echo ***********************************
exit /b 0

:BuildError
echo ***********************************
echo ********** Build Failed ***********
echo ***********************************
pause
exit /b 1
