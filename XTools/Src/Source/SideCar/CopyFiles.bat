@echo off
set src_folder=%1
set dst_folder=%2
set file_list=%3

for /f "tokens=*" %%i in (%file_list%) DO xcopy "%src_folder%%%i" "%dst_folder%%%i*" /F /H /R /Y
