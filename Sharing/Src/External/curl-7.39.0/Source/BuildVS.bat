@echo off

call "%VS140COMNTOOLS%vsvars32.bat"

rem call MSBuild projects\Windows\VC14\lib\libcurl.sln /p:Configuration="LIB Debug";Platform=Win32 /t:rebuild /m
rem IF NOT %ERRORLEVEL% == 0 goto BuildError
rem 
rem call MSBuild projects\Windows\VC14\lib\libcurl.sln /p:Configuration="LIB Release";Platform=Win32 /t:rebuild /m
rem IF NOT %ERRORLEVEL% == 0 goto BuildError
rem 
rem call MSBuild projects\Windows\VC14\lib\libcurl.sln /p:Configuration="LIB Debug";Platform=x64 /t:rebuild /m
rem IF NOT %ERRORLEVEL% == 0 goto BuildError
rem 
rem call MSBuild projects\Windows\VC14\lib\libcurl.sln /p:Configuration="LIB Release";Platform=x64 /t:rebuild /m
rem IF NOT %ERRORLEVEL% == 0 goto BuildError

tf checkout ..\lib -r
tf checkout ..\include -r

xcopy "build\LIB Debug\x64\*.lib" ..\lib\Debug\x64\ /I /F /H /R /K /Y
xcopy "build\LIB Debug\x64\*.pdb" ..\lib\Debug\x64\ /I /F /H /R /K /Y

xcopy "build\LIB Release\x64\*.lib" ..\lib\Release\x64\ /I /F /H /R /K /Y
xcopy "build\LIB Release\x64\*.pdb" ..\lib\Release\x64\ /I /F /H /R /K /Y

xcopy "build\LIB Debug\Win32\*.lib" ..\lib\Debug\Win32\ /I /F /H /R /K /Y
xcopy "build\LIB Debug\Win32\*.pdb" ..\lib\Debug\Win32\ /I /F /H /R /K /Y

xcopy "build\LIB Release\Win32\*.lib" ..\lib\Release\Win32\ /I /F /H /R /K /Y
xcopy "build\LIB Release\Win32\*.pdb" ..\lib\Release\Win32\ /I /F /H /R /K /Y

xcopy include\curl\*.h ..\include\ /I /F /H /R /K /Y


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
