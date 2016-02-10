@echo off

echo ************************************************
echo ********* Building XTools Dependencies *********
echo ************************************************
echo
echo

call apriltag-2015-03-18\BuildLibs.bat
IF NOT %ERRORLEVEL% == 0 goto BuildError

call curl-7.39.0\Source\BuildAll.bat
IF NOT %ERRORLEVEL% == 0 goto BuildError

call RakNet-4.081\src\BuildLibs.bat
IF NOT %ERRORLEVEL% == 0 goto BuildError

call speex-1.2rc1\BuildLibs.bat
IF NOT %ERRORLEVEL% == 0 goto BuildError

call zint-2.4.3\src\BuildLibs.bat
IF NOT %ERRORLEVEL% == 0 goto BuildError

call zxing-cpp\src\BuildLibs.bat
IF NOT %ERRORLEVEL% == 0 goto BuildError

:End
echo ************************************************
echo ********* Dependencies Build Succeeded *********
echo ************************************************
exit /b 0

:BuildError
echo ************************************************
echo ********** Dependencies Build Failed ***********
echo ************************************************
pause
exit /b 1
