echo Copy XTools Plugin
xcopy %1ClientWindowsJava\XToolsClient.dll %2 /S /I /F /H /R /K /Y
xcopy %1ClientWindowsJava\XToolsClient.pdb %2 /S /I /F /H /R /K /Y

echo Copy SideCar add-in
xcopy %1SideCar.Test\SideCar.Test.dll %2\SideCars\ /S /I /F /H /R /K /Y
xcopy %1SideCar.Test\SideCar.Test.pdb %2\SideCars\ /S /I /F /H /R /K /Y
