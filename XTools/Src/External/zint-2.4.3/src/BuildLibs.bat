@echo off

call "%VS140COMNTOOLS%vsvars32.bat"

call MSBuild win32/zint.sln /p:Configuration=Debug;Platform=ARM /t:rebuild /m
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild win32/zint.sln /p:Configuration=Release;Platform=ARM /t:rebuild /m
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild win32/zint.sln /p:Configuration=Debug;Platform=Win32 /t:rebuild /m
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild win32/zint.sln /p:Configuration=Release;Platform=Win32 /t:rebuild /m
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild win32/zint.sln /p:Configuration=Debug;Platform=x64 /t:rebuild /m
IF NOT %ERRORLEVEL% == 0 goto BuildError

call MSBuild win32/zint.sln /p:Configuration=Release;Platform=x64 /t:rebuild /m
IF NOT %ERRORLEVEL% == 0 goto BuildError

xcopy build\* ..\lib\ /I /F /H /R /K /Y /S

xcopy backend\*.h ..\include\ /I /F /H /R /K /Y /S
xcopy backend\*.hh ..\include\ /I /F /H /R /K /Y /S

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
