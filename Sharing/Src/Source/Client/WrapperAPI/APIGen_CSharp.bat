
echo Building CSharp Wrapper API...

set DEST=..\..\..\GeneratedInterface\CSharpAPI

if exist %DEST% del /F %DEST%\*.cs
if not exist %DEST% mkdir %DEST%

..\..\..\External\swig\swig.exe -Fmicrosoft -c++ -csharp -D%2 -D%3 %5 -namespace HoloToolkit.Sharing -dllimport SharingClient -outdir %DEST% -o ..\ClientWrapperAPI_CSharp.cpp -includeall %4
IF NOT %ERRORLEVEL% == 0 goto BuildError

echo ...Completed Successfully
exit /b 0

:BuildError
echo ***********************************
echo * CSharp Wrapper API Build Failed *
echo ***********************************
exit /b 1
