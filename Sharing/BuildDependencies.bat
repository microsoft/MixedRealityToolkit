@echo off

echo *************************************************************
echo ********* Building HoloToolkit.Sharing Dependencies *********
echo *************************************************************
echo
echo

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
