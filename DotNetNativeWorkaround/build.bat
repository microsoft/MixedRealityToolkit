@echo off

REM Run this from the Developer Command Prompt for VS

call MSBuild DotNetNativeWorkaround.sln /p:Configuration=Release;Platform=x86 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild DotNetNativeWorkaround.sln /p:Configuration=Release;Platform=x64 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild DotNetNativeWorkaround.sln /p:Configuration=Release;Platform=ARM /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild DotNetNativeWorkaround.sln /p:Configuration=Release;Platform=ARM64 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

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
