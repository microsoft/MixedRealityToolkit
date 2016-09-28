@echo off

call "%VS140COMNTOOLS%vsvars32.bat"

call MSBuild RakNet.sln /p:Configuration=Debug;Platform=ARM /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild RakNet.sln /p:Configuration=Release;Platform=ARM /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild RakNet.sln /p:Configuration=Debug;Platform=Win32 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild RakNet.sln /p:Configuration=Release;Platform=Win32 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild RakNet.sln /p:Configuration=Debug;Platform=x64 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild RakNet.sln /p:Configuration=Release;Platform=x64 /m %*
IF NOT %ERRORLEVEL% == 0 goto BuildError

xcopy Lib\*.lib ..\lib\ /I /F /H /R /K /Y
xcopy Lib\*.pdb ..\lib\ /I /F /H /R /K /Y
xcopy Lib\*.idb ..\lib\ /I /F /H /R /K /Y
xcopy Lib\*.pri ..\lib\ /I /F /H /R /K /Y

xcopy Source\*.h ..\include\Source\ /I /F /H /R /K /Y

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
