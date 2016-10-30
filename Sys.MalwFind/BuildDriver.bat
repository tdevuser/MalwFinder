

SET WDK_ROOT=E:\WinDDK\7600.16385.1
SET DRIVER_ROOT=%~dp0%

rem nmake -f makefile.mak rebuild
nmake -f makefile.mak debug

rem pushd %DRIVER_ROOT%\bin\DriverSign && DriverSign.bat

pause

