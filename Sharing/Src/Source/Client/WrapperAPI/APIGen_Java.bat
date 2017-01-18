echo Building Java Wrapper API...

echo In order to function correctly, please ensure the following environment variables are correctly set: 
echo JAVA_INCLUDE: %JAVA_INCLUDE% 
echo JAVA_BIN: %JAVA_BIN% 
echo on 

set DEST=..\..\..\GeneratedInterface\JavaAPI\com\microsoft\holotoolkit\sharing

if exist %DEST% del /F %DEST%\*.java
if not exist %DEST% mkdir %DEST%

..\..\..\External\swig\swig.exe -Fmicrosoft -c++ -java -D%1 -D%2 %4 -package com.microsoft.holotoolkit.sharing -outdir %DEST% -o ..\ClientWrapperAPI_Java.cpp -includeall %3
IF NOT %ERRORLEVEL% == 0 goto BuildError

echo ...Completed Successfully
exit /b 0

:BuildError
echo ***********************************
echo ** Java Wrapper API Build Failed **
echo ***********************************
exit /b 1
