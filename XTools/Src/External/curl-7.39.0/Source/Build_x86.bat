@echo off

REM vsvars32.bat appears to have issues if you call it too many times from one command prompt.
REM We'll use "DevEnvDir" to determine if we already ran it or if we need to run it again.
@if "%DevEnvDir%"=="" call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" x86

pushd "%~dp0"

cd winbuild

nmake /f Makefile.vc mode=static VC=14 DEBUG=yes MACHINE=x86 GEN_PDB=yes RTLIBCFG=static
IF NOT %ERRORLEVEL% == 0 goto BuildError

nmake /f Makefile.vc mode=static VC=14 DEBUG=no MACHINE=x86 GEN_PDB=yes RTLIBCFG=static
IF NOT %ERRORLEVEL% == 0 goto BuildError

:End
echo ***********************************
echo ********* Build Succeeded *********
echo ***********************************
popd
exit /b 0

:BuildError
echo ***********************************
echo ********** Build Failed ***********
echo ***********************************
pause
popd
exit /b 1

