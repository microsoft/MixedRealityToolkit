@echo off

REM vsvars32.bat appears to have issues if you call it too many times from one command prompt.
REM We'll use "DevEnvDir" to determine if we already ran it or if we need to run it again.
@if "%DevEnvDir%"=="" call "%VS140COMNTOOLS%vsvars32.bat"

call MSBuild Src\Solutions\VisualStudio\HoloToolkit.Sharing.sln /p:Configuration=Debug;Platform=ARM /t:rebuild /m
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild Src\Solutions\VisualStudio\HoloToolkit.Sharing.sln /p:Configuration=Release;Platform=ARM /t:rebuild /m
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild Src\Solutions\VisualStudio\HoloToolkit.Sharing.sln /p:Configuration=Debug;Platform=Win32 /t:rebuild /m
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild Src\Solutions\VisualStudio\HoloToolkit.Sharing.sln /p:Configuration=Release;Platform=Win32 /t:rebuild /m
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild Src\Solutions\VisualStudio\HoloToolkit.Sharing.sln /p:Configuration=Debug;Platform=x64 /t:rebuild /m
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild Src\Solutions\VisualStudio\HoloToolkit.Sharing.sln /p:Configuration=Release;Platform=x64 /t:rebuild /m
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
