@echo off

call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" amd64

pushd "%~dp0"

cd winbuild

nmake /f Makefile.vc mode=static VC=14 DEBUG=yes MACHINE=x64 GEN_PDB=yes RTLIBCFG=static
IF NOT %ERRORLEVEL% == 0 goto BuildError

nmake /f Makefile.vc mode=static VC=14 DEBUG=no MACHINE=x64 GEN_PDB=yes RTLIBCFG=static
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

