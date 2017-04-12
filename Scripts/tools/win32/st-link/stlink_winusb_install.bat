@echo off

REM First ensure the command runs with enough rights

openfiles > NUL 2>&1 
if %errorlevel%==0 goto ADMIN 
echo Installing ST-Link drivers requires elevated rights; launch the command prompt in administrator mode and/or change user account 
pause
goto END 

:ADMIN

REM If enough rights, check the machine architecture

wmic OS get OSArchitecture | findstr 64-bit > NUL 2>&1 
if %errorlevel%==1 goto X86
dpinst_amd64.exe
goto END

:X86
dpinst_x86.exe

:END
