@echo off

echo ***********************************
echo ********** Building Docs **********
echo ***********************************

if not exist Src\External\doxygen\bin\doxygen.exe (
    echo Doxygen not found.  Please download Doxygen and place it here: Src\External\doxygen\bin\doxygen.exe
	pause
	exit /b 1
) 

rmdir /s /q Src\Docs
mkdir Src\Docs\C++

pushd Src\Source\Docs

..\..\External\doxygen\bin\doxygen.exe Doxyfile.txt

popd

echo ******************************************
echo ********** Build Docs Succeeded **********
echo ******************************************
exit /b 0
