@echo off

echo ************************************************
echo ********* Building XTools Dependencies *********
echo ************************************************
echo
echo

pushd apriltag-2015-03-18
call BuildLibs.bat
IF NOT %ERRORLEVEL% == 0 goto BuildError
popd

pushd curl-7.39.0\Source
call BuildAll.bat
IF NOT %ERRORLEVEL% == 0 goto BuildError
popd

pushd RakNet-4.081\src
call BuildLibs.bat
IF NOT %ERRORLEVEL% == 0 goto BuildError
popd

pushd speex-1.2rc1
call BuildLibs.bat
IF NOT %ERRORLEVEL% == 0 goto BuildError
popd

pushd zint-2.4.3\src
call BuildLibs.bat
IF NOT %ERRORLEVEL% == 0 goto BuildError
popd

pushd zxing-cpp\src
call BuildLibs.bat
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
