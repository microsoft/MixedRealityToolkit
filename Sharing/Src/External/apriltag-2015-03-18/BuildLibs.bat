rem @echo off

call "%VS140COMNTOOLS%vsvars32.bat"

pushd src\vs

call MSBuild apriltag.sln /p:Configuration=Debug;Platform=Win32 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild apriltag.sln /p:Configuration=Release;Platform=Win32 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild apriltag.sln /p:Configuration=Debug;Platform=x64 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild apriltag.sln /p:Configuration=Release;Platform=x64 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild apriltag.sln /p:Configuration=Debug;Platform=ARM /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild apriltag.sln /p:Configuration=Release;Platform=ARM /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

popd

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
