echo Copy XTools Plugin to bin
xcopy %1ClientWindowsCSharp\XToolsClient.dll %2 /S /I /F /H /R /K /Y /C
xcopy %1ClientWindowsCSharp\XToolsClient.pdb %2 /S /I /F /H /R /K /Y /C

exit /B 0