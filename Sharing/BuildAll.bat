@echo off

echo ****************************************************
echo ********* Building All HoloToolkit.Sharing *********
echo ****************************************************

call BuildDependencies.bat %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call BuildSharingLib.bat %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call CopyToSDK.bat
IF NOT %ERRORLEVEL% == 0 goto BuildError

:End
echo ***************************************
echo ********* Build All Succeeded *********
echo ***************************************
exit /b 0

:BuildError
echo ***************************************
echo ********** Build All Failed ***********
echo ***************************************
exit /b 1
