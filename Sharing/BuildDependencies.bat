@echo off

echo *************************************************************
echo ********* Building HoloToolkit.Sharing Dependencies *********
echo *************************************************************
echo
echo

pushd Src\External\apriltag-2015-03-18
call BuildLibs.bat %*
IF NOT %ERRORLEVEL% == 0 goto BuildError
popd

REM No %* for curl - it doesn't use msbuild so can't forward all args
pushd Src\External\curl-7.39.0\Source
call BuildAll.bat
IF NOT %ERRORLEVEL% == 0 goto BuildError
popd

pushd Src\External\RakNet-4.081\src
call BuildLibs.bat %*
IF NOT %ERRORLEVEL% == 0 goto BuildError
popd

pushd Src\External\speex-1.2rc1
call BuildLibs.bat %*
IF NOT %ERRORLEVEL% == 0 goto BuildError
popd

pushd Src\External\zint-2.4.3\src
call BuildLibs.bat %*
IF NOT %ERRORLEVEL% == 0 goto BuildError
popd

pushd Src\External\zxing-cpp\src
call BuildLibs.bat %*
IF NOT %ERRORLEVEL% == 0 goto BuildError
popd

:End
echo ************************************************
echo ********* Dependencies Build Succeeded *********
echo ************************************************
exit /b 0

:BuildError
echo ************************************************
echo ********** Dependencies Build Failed ***********
echo ************************************************
exit /b 1
