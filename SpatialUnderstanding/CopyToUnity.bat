@echo off
@setlocal

if not exist "%HoloToolkitUnityRoot%" set HoloToolkitUnityRoot=%1
if "x%HoloToolkitUnityRoot%" == "x" goto ErrorNoHoloToolkitUnityTarget
if not exist "%HoloToolkitUnityRoot%\Assets" goto ErrorHoloToolkitUnityTargetInvalid

xcopy Src\BuildOutput\bin\StandaloneRelease\Win32\SpatialUnderstanding\SpatialUnderstanding.dll "%HoloToolkitUnityRoot%\Assets\HoloToolkit\SpatialUnderstanding\Plugins\x86\" /S /I /F /H /R /K /Y
xcopy Src\BuildOutput\bin\StandaloneRelease\Win32\SpatialUnderstanding\SpatialUnderstanding.pdb "%HoloToolkitUnityRoot%\Assets\HoloToolkit\SpatialUnderstanding\Plugins\x86\" /S /I /F /H /R /K /Y
xcopy Src\BuildOutput\bin\StandaloneRelease\x64\SpatialUnderstanding\SpatialUnderstanding.dll "%HoloToolkitUnityRoot%\Assets\HoloToolkit\SpatialUnderstanding\Plugins\x64\" /S /I /F /H /R /K /Y
xcopy Src\BuildOutput\bin\StandaloneRelease\x64\SpatialUnderstanding\SpatialUnderstanding.pdb "%HoloToolkitUnityRoot%\Assets\HoloToolkit\SpatialUnderstanding\Plugins\x64\" /S /I /F /H /R /K /Y
xcopy Src\BuildOutput\bin\UniversalRelease\Win32\SpatialUnderstanding\SpatialUnderstanding\SpatialUnderstanding.dll "%HoloToolkitUnityRoot%\Assets\HoloToolkit\SpatialUnderstanding\Plugins\WSA\x86\" /S /I /F /H /R /K /Y
xcopy Src\BuildOutput\bin\UniversalRelease\Win32\SpatialUnderstanding\SpatialUnderstanding\SpatialUnderstanding.pdb "%HoloToolkitUnityRoot%\Assets\HoloToolkit\SpatialUnderstanding\Plugins\WSA\x86\" /S /I /F /H /R /K /Y
xcopy Src\BuildOutput\bin\UniversalRelease\x64\SpatialUnderstanding\SpatialUnderstanding\SpatialUnderstanding.dll "%HoloToolkitUnityRoot%\Assets\HoloToolkit\SpatialUnderstanding\Plugins\WSA\x64\" /S /I /F /H /R /K /Y
xcopy Src\BuildOutput\bin\UniversalRelease\x64\SpatialUnderstanding\SpatialUnderstanding\SpatialUnderstanding.pdb "%HoloToolkitUnityRoot%\Assets\HoloToolkit\SpatialUnderstanding\Plugins\WSA\x64\" /S /I /F /H /R /K /Y

exit /b 0

:ErrorNoHoloToolkitUnityTarget
echo Target path is required. Set the HoloToolkitUnityRoot environment variable or pass the root of the target Unity project as the first argument to this script.
exit /b 1

:ErrorHoloToolkitUnityTargetInvalid
echo Target path is not a valid Unity project, verify that the Assets and External directories exists.
exit /b 1