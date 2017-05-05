@echo off

REM vsvars32.bat appears to have issues if you call it too many times from one command prompt.
REM We'll use "DevEnvDir" to determine if we already ran it or if we need to run it again.
@if "%DevEnvDir%"=="" call "%VS140COMNTOOLS%vsvars32.bat"

call MSBuild Src\SpatialUnderstanding.sln /p:Configuration=StandaloneRelease;Platform=x86 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild Src\SpatialUnderstanding.sln /p:Configuration=StandaloneRelease;Platform=x64 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild Src\SpatialUnderstanding.sln /p:Configuration=UniversalRelease;Platform=x86 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild Src\SpatialUnderstanding.sln /p:Configuration=UniversalRelease;Platform=x64 /m %*
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
